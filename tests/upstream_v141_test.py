#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Pinned maintenance audit and real installer/menu upgrades on temporary host units.

No device/network commands, executable firmware or real /mnt writes. Installer
copies are redirected below /tmp, remount/sync calls disabled; payload files are
inert text for exercising copy/upgrade behavior, never product implementations.
Run from any directory: python3 tests/upstream_v141_test.py
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
TARGET = "b127988a68c8d690841f4a75d35717ffc01e668e"
BASE = "5c460600c9205cdafa2c4fecd7e2210b8606aedb"
CUSTOM = "6d5e66021b5f511e499da35557af67bdf213b75d"
MAINTENANCE = "f5bd2e72290a1509d63f831e7dce4dc5282afaa9"
INSTALL = "mods/install_MoreIncredibleBash/mod/custom.sh"
MENU = "mods/menu_install_MoreIncredibleBash/mod/custom.sh"
DELAY = "altscreen_render.hwdecode.delayed"
ASSETS = (
    "libcarplay_hook.so maneuver_render flag_atlas.rgba carplay_startup.sh "
    "carplay_monitor.sh carplay_processes.sh carplay_cleanup.sh carplay_hook.jar "
    "libaa_cluster_hook.so aa_startup.sh libaltscreen111_mhi2q.so altscreen_render"
).split()
STOCK_SI = '''{
    "children": {
        "gal":{
            "exec":"gal",
            "path":"/mnt/app/eso/bin/apps"
        },
        "carplay": {
            "exec": "dio_manager",
            "path": "/mnt/app/eso/bin/apps"
        }
    }
}
'''
STOCK_DIO = '''        "nmea":"0x0003",
        "MessagesSentByAccessory":["0x5000", "0x4C05"],
        "MessagesReceivedFromDevice":["0x5001", "0x4C04"],
'''


def git(*args):
    return subprocess.check_output(["git", "-C", str(ROOT), *args])


def source(path, old=False):
    return git("show", f"{BASE}:{path}").decode() if old else (ROOT / path).read_text()


def run(*args, env=None):
    result = subprocess.run(args, env=env, capture_output=True, text=True, timeout=30)
    if result.returncode:
        raise AssertionError(f"{args}: exit {result.returncode}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def redirected(text, unit):
    # Same host-fixture strategy as upstream test_install_payload.sh. The actual
    # JSON patch/copy/backup/menu functions are kept intact.
    text = text.replace("/mnt/", f"{unit}/mnt/")
    text = text.replace("/tmp/carplay_files.", f"{unit}/tmp/carplay_files.")
    text = re.sub(r"(?m)^(\s*)mount -uw", r"\1: mount -uw", text)
    return re.sub(r"(?m)^(\s*)sync\s*$", r"\1: sync", text)


class Upstream141Test(unittest.TestCase):
    def test_release_already_integrated(self):
        self.assertEqual(git("rev-parse", "v1.4.1^{}").decode().strip(), TARGET)
        self.assertEqual(git("rev-parse", "v1.4^{}").decode().strip(), BASE)
        git("merge-base", "--is-ancestor", TARGET, "HEAD")
        changed = git("diff", "--name-only", BASE, TARGET).decode().splitlines()
        self.assertEqual(len(changed), 20)
        for path in changed:
            with self.subTest(path=path):
                self.assertEqual(git("hash-object", str(ROOT / path)).strip(),
                                 git("rev-parse", f"{TARGET}:{path}").strip())

    def test_maintenance_commit_preserved_dashboard_and_roadmap(self):
        # This historical preservation assertion belongs to the maintenance
        # integration, not every future dashboard revision. Functional suites
        # cover the current feature tree; do not freeze it to its P2-02 state.
        git("merge-base", "--is-ancestor", MAINTENANCE, "HEAD")
        paths = ["dashboard", "tests/dashboard_test.c", "tests/dashboard_render_test.c",
                 "tests/dashboard_svg_test.py", "docs/ROADMAP.md",
                 "docs/custom-dashboards/data-providers.md"]
        self.assertEqual(git("diff", CUSTOM, MAINTENANCE, "--", *paths), b"")

    def test_renderer_hardware_policy_on_host(self):
        # Extract real policy functions; only filesystem paths and the clock are
        # fixtures. This does NOT emulate OMX, Screen, QNX IPC or video hardware.
        text = source("altscreen_render/main.c")

        def function(name):
            match = re.search(r"^static [^\n]*\b" + name + r"\([^\n]*\)\s*\n\{.*?^\}", text, re.M | re.S)
            self.assertIsNotNone(match, name)
            return match.group()

        with tempfile.TemporaryDirectory(prefix="mib2q-v141-hw-", dir="/tmp") as tmp:
            tmp = Path(tmp)
            # All paths used by extracted code are below the disposable test root.
            names = ("ALTR_HWDEC_OFF_MARKER", "ALTR_HWDEC_OFF_BOOT", "ALTR_AA_HWDEC_OFF_MARKER",
                     "ALTR_AA_HWDEC_OFF_BOOT", "ALTR_HW_SETTLE_MARKER", "ALTR_HW_YIELD_FILE", "ALTR_HW_ACTIVE_FILE")
            lines = [f'#define {name} "{tmp}/{name}"' for name in names]
            for name in ("ALTR_HW_YIELD_POLL_MS", "ALTR_HW_START_MAX", "ALTR_HW_COOLDOWN_MS", "ALTR_HW_ROUNDS_MAX"):
                lines.append(re.search(r"^#define " + name + r"\s+.*$", text, re.M).group())
            lines += [re.search(r"^struct source \{.*?^\};", text, re.M | re.S).group(),
                      re.search(r"^struct hw_starts \{.*?; \};", text, re.M).group(),
                      re.search(r"^static uint64_t g_hw_yield_hold_ms =.*$", text, re.M).group(),
                      next(line for line in text.splitlines() if line.startswith("static int env_i(")),
                      "static uint64_t clock_ms = 1000; static uint64_t now_ms(void) { return clock_ms; }"]
            for name in ("sources_init", "hw_wanted", "hw_switch_off", "hw_yield_until", "hw_active_mark",
                         "hw_yield_claim", "hw_starts_path", "hw_starts_load", "hw_starts_save",
                         "hw_starts_clear", "hw_starts_begin"):
                lines.append(function(name))
            settle = re.search(r"int settle = [^\n]+;", text).group()
            condition = re.search(r"if \((settle && now_ms\(\) < a.hw_min_uptime_ms)\)", text).group(1)
            lines.append("static int delayed(const struct source *s) { struct { uint64_t hw_min_uptime_ms; } a={120000}; "
                         + settle + " return " + condition + "; }")
            harness = r'''
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "decode.h"
'''+"\n".join(lines)+r'''
static void put(const char *path, unsigned long long value) {
    FILE *f=fopen(path,"w"); assert(f); assert(fprintf(f,"%llu\n",value)>0); assert(fclose(f)==0);
}
int main(void) {
    struct source s[2]; int n, i; uint64_t until=0; char record[160];
    sources_init(s,&n,1440,540); assert(n==2);
    assert(s[0].port==19820 && s[1].port==19821);
    assert(s[0].sw_threads==1 && s[1].sw_threads==2);
    assert(s[0].hw_fail_persists==1 && s[1].hw_fail_persists==0);
    assert(s[1].live_timeout_ms==0);
    assert(!strcmp(s[0].hw_settle_marker,s[1].hw_settle_marker));
    assert(!delayed(&s[0]) && !delayed(&s[1]));
    put(ALTR_HW_SETTLE_MARKER,1); assert(delayed(&s[0]) && delayed(&s[1]));
    clock_ms=120000; assert(!delayed(&s[0]) && !delayed(&s[1]));
    unlink(ALTR_HW_SETTLE_MARKER);
    assert(hw_wanted(&s[0]) && hw_wanted(&s[1]));
    hw_switch_off(&s[1]); assert(!hw_wanted(&s[1]) && hw_wanted(&s[0]));
    assert(access(s[1].hw_off_marker,F_OK)!=0 && access(s[1].hw_off_boot,F_OK)==0);
    unlink(s[1].hw_off_boot);
    hw_switch_off(&s[0]); assert(access(s[0].hw_off_marker,F_OK)==0 && !hw_wanted(&s[0]));
    unlink(s[0].hw_off_boot); unlink(s[0].hw_off_marker);
    clock_ms=1000; put(ALTR_HW_YIELD_FILE,1000);
    assert(hw_yield_claim(&until)==1 && until==4000);
    assert(access(ALTR_HW_ACTIVE_FILE,F_OK)!=0);
    clock_ms=3999; assert(hw_yield_until(1)==4000);
    clock_ms=4000; assert(hw_yield_until(1)==0);
    assert(hw_yield_claim(&until)==0 && access(ALTR_HW_ACTIVE_FILE,F_OK)==0);
    hw_active_mark(0);
    for(i=0;i<3;i++) assert(hw_starts_begin(&s[1],1000,&until)==0);
    assert(hw_starts_begin(&s[1],1000,&until)==1 && until==31000);
    assert(hw_starts_begin(&s[1],30999,&until)==1);
    for(i=0;i<3;i++) assert(hw_starts_begin(&s[1],31000,&until)==0);
    assert(hw_starts_begin(&s[1],31000,&until)==-1 && !hw_wanted(&s[1]));
    assert(hw_wanted(&s[0]));
    hw_starts_clear(&s[1]); hw_starts_path(&s[1],record,sizeof(record));
    assert(access(record,F_OK)!=0);
    puts("renderer policy: immediate/opt-in delay, per-source fallback, yield/claim/hold, cooldown/recovery PASS");
    return 0;
}
'''
            cfile = tmp / "policy.c"
            cfile.write_text(harness)
            run("cc", "-std=gnu99", "-O1", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "altscreen_render"), str(cfile), "-o", str(tmp / "policy"))
            env = {k: v for k, v in os.environ.items() if not k.startswith("ALTR_")}
            run(str(tmp / "policy"), env=env)

    def test_upgrade_preserves_preferences_and_original_backups(self):
        shells = [p for p in ("/bin/ksh", "/bin/mksh", "/bin/dash", "/bin/sh")
                  if os.access(p, os.X_OK)]
        self.assertTrue(shells)
        with tempfile.TemporaryDirectory(prefix="mib2q-v141-", dir="/tmp") as tmp:
            tmp = Path(tmp)
            # Generate each version's actual GEM menu under a disposable project.
            menus = {}
            for old in (True, False):
                stage = tmp / ("old-menu" if old else "new-menu")
                (stage / "scripts").mkdir(parents=True)
                for src in ROOT.glob("mods/*/mod/custom.sh"):
                    path = src.relative_to(ROOT)
                    dest = stage / path
                    dest.parent.mkdir(parents=True, exist_ok=True)
                    dest.write_text(source(str(path), old))
                builder = stage / "scripts/build_gem_menu.sh"
                builder.write_text(source("scripts/build_gem_menu.sh", old))
                dispatcher = stage / "deploy/gem_menu/run.sh"
                dispatcher.parent.mkdir(parents=True)
                dispatcher.write_text(source("deploy/gem_menu/run.sh", old))
                run("bash", str(builder))
                menus[old] = stage / "mods/menu_install_MoreIncredibleBash/mod/carplay_menu"

            for i, shell in enumerate(shells):
                for delayed in (False, True):
                    with self.subTest(shell=shell, delayed=delayed):
                        unit = tmp / f"unit-{i}-{int(delayed)}"
                        payload = unit / "card/mod/carplay"
                        payload.mkdir(parents=True)
                        (unit / "tmp").mkdir()
                        cfg = unit / "mnt/system/etc/eso/production"
                        cfg.mkdir(parents=True)
                        (cfg / "smartphone_integrator.json").write_text(STOCK_SI)
                        (cfg / "dio_manager.json").write_text(STOCK_DIO)
                        gem = unit / "mnt/app/eso/hmi/engdefs"
                        gem.mkdir(parents=True)
                        prefs = unit / "mnt/app/root"
                        prefs.mkdir(parents=True)
                        env = dict(os.environ, CP_INSTALL_RES=str(payload), CP_GEM_DIR=str(gem))
                        env.pop("LD_PRELOAD", None)

                        def install(old, action="install"):
                            version = "1.4" if old else "1.4.1"
                            for asset in ASSETS:
                                (payload / asset).write_text(f"{version}:{asset}\n")
                            for child in ("carplay_child.json", "aa_child.json"):
                                (payload / child).write_text(source(f"deploy/smartphone_integrator/{child}", old))
                            script = unit / "card/mod/custom.sh"
                            script.write_text(redirected(source(INSTALL, old), unit))
                            run(shell, str(script), action, env=env)

                        def menu(old):
                            folder = unit / "card/menu"
                            folder.mkdir(exist_ok=True)
                            shutil.copytree(menus[old], folder / "carplay_menu", dirs_exist_ok=True)
                            script = folder / "custom.sh"
                            script.write_text(redirected(source(MENU, old), unit))
                            run(shell, str(script), env=env)

                        install(True)
                        menu(True)
                        self.assertNotIn("CarPlay + Android Auto, default OFF", (gem / "mib2q-carplay.esd").read_text())
                        preferences = {"carplay_vehicle_engine": b"diesel\n",
                                       "mibr-carplay111-nav.eta": b"yes\n",
                                       "altscreen_render.aa.hwdecode.off": b""}
                        if delayed:
                            preferences[DELAY] = b"user-enabled-before-upgrade\n"
                        for name, contents in preferences.items():
                            (prefs / name).write_bytes(contents)
                        for _ in range(2):  # upgrade, then idempotent reinstall
                            install(False)
                            menu(False)
                            self.assertEqual((prefs / DELAY).exists(), delayed)
                            for name, contents in preferences.items():
                                self.assertEqual((prefs / name).read_bytes(), contents)
                            for name, contents in (("smartphone_integrator.json", STOCK_SI),
                                                   ("dio_manager.json", STOCK_DIO)):
                                self.assertEqual((cfg / (name + ".carplay-stock")).read_text(), contents)
                            label = (gem / "mib2q-carplay.esd").read_text()
                            self.assertIn("HW decoder 2-min delayed start ON / OFF (toggle, CarPlay + Android Auto, default OFF)", label)
                            hooks = prefs / "hooks"
                            self.assertEqual((hooks / "libaa_cluster_hook.so").read_text(), "1.4.1:libaa_cluster_hook.so\n")
                            self.assertEqual((hooks / "altscreen_render").read_text(), "1.4.1:altscreen_render\n")
                        install(False, "uninstall")
                        self.assertEqual((cfg / "smartphone_integrator.json").read_text(), STOCK_SI)
                        self.assertEqual((cfg / "dio_manager.json").read_text(), STOCK_DIO)
                        self.assertFalse((prefs / "hooks/libaa_cluster_hook.so").exists())


if __name__ == "__main__":
    unittest.main(verbosity=2)
