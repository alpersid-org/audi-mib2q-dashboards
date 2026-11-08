#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Real installer on inert host payloads: errors, interrupted updates and rollback.

All /mnt paths are redirected into a TemporaryDirectory. No device access,
remounts, executable firmware or real system configuration changes.
"""
from pathlib import Path
import os
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
INSTALL = ROOT / 'mods/install_MoreIncredibleBash/mod/custom.sh'
UNINSTALL = ROOT / 'mods/uninstall_MoreIncredibleBash/mod/custom.sh'
ASSETS = ('libcarplay_hook.so maneuver_render flag_atlas.rgba carplay_startup.sh '
          'carplay_monitor.sh carplay_processes.sh carplay_cleanup.sh carplay_hook.jar').split()
CONFLICTS = ('NavActiveIgnore.jar', 'navignore_audi.jar', 'navignore_vw.jar')
SI = '''{
    "children": {
        "carplay": {
            "exec": "dio_manager",
            "path": "/mnt/app/eso/bin/apps"
        }
    }
}
'''
DIO = '''        "nmea":"0x0003",
        "MessagesSentByAccessory":["0x5000", "0x4C05"],
        "MessagesReceivedFromDevice":["0x5001", "0x4C04"],
'''
SHELLS = [p for p in ('/bin/ksh', '/bin/mksh', '/bin/dash', '/bin/bash') if os.access(p, os.X_OK)]


class InstallerRecoveryTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='mib2q-install-recovery-')
        self.addCleanup(self.tmp.cleanup)
        self.unit = Path(self.tmp.name)
        self.hooks = self.unit / 'mnt/app/root/hooks'
        self.jars = self.unit / 'mnt/app/eso/hmi/lsd/jars'
        self.config = self.unit / 'mnt/system/etc/eso/production'
        self.payload = self.unit / 'card/carplay'
        for p in (self.hooks, self.jars, self.config, self.payload, self.unit / 'tmp'):
            p.mkdir(parents=True, exist_ok=True)
        for name in ASSETS:
            (self.payload / name).write_text('new:' + name)
            ((self.jars if name.endswith('.jar') else self.hooks) / name).write_text('old:' + name)
        for name in CONFLICTS:
            (self.jars / name).write_bytes(('original:' + name).encode())
        (self.payload / 'carplay_child.json').write_bytes(
            (ROOT / 'deploy/smartphone_integrator/carplay_child.json').read_bytes())
        (self.config / 'smartphone_integrator.json').write_text(SI)
        (self.config / 'dio_manager.json').write_text(DIO)
        self.tx = self.hooks / '.carplay-transaction'

    def snapshot(self):
        return {str(p.relative_to(self.unit)): p.read_bytes()
                for p in (self.unit / 'mnt').rglob('*') if p.is_file()}

    def script(self, source=INSTALL, fault=None):
        text = source.read_text().replace('/mnt/', str(self.unit / 'mnt') + '/')
        text = text.replace('/tmp/carplay_files.', str(self.unit / 'tmp/carplay_files.'))
        text = text.replace('/tmp/carplay_stock.', str(self.unit / 'tmp/carplay_stock.'))
        text = re.sub(r'(?m)^(\s*)mount -uw', r'\1: mount -uw', text)
        text = re.sub(r'(?m)^(\s*)sync(?: \|\| (?:exit|return) 1)?$', r'\1:', text)
        if fault in ('kill', 'term'):
            signal = 'KILL' if fault == 'kill' else 'TERM'
            text = text.replace('    patch_json ||', '    kill -' + signal + ' $$\n    patch_json ||')
        elif fault == 'committed-kill':
            text = text.replace('    tx_active=0\n    rm -rf', '    kill -KILL $$\n    tx_active=0\n    rm -rf')
        elif fault in ('stage', 'activate', 'rollback'):
            pattern = '*.new' if fault == 'stage' else '*.carplay-new.*'
            # Only cp is injected. Destination operations and transaction logic
            # remain production code. The failure marker also lives in the fixture.
            wrapper = '''
copy_count=0
cp() {
    for copy_dest do :; done
    case $copy_dest in PATTERN)
        copy_count=$((copy_count + 1))
        if [ "$copy_count" = 2 ]; then return 1; fi ;;
    esac
    ROLLBACK_FAILURE
    command cp "$@"
}
'''.replace('PATTERN', pattern).replace('ROLLBACK_FAILURE',
                'case $copy_dest in *.carplay-restore.*) return 1 ;; esac' if fault == 'rollback' else ':')
            text = text.replace('unset LD_PRELOAD', 'unset LD_PRELOAD\n' + wrapper)
        path = self.unit / ('uninstall.sh' if source == UNINSTALL else 'install.sh')
        path.write_text(text)
        return path

    def run_script(self, shell, action='install', fault=None, source=INSTALL):
        env = dict(os.environ, CP_INSTALL_RES=str(self.payload))
        return subprocess.run([shell, str(self.script(source, fault)), action], env=env,
                              capture_output=True, text=True, timeout=30)

    def assert_failed_and_restored(self, shell, fault=None):
        before = self.snapshot()
        result = self.run_script(shell, fault=fault)
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn('DONE (install)', result.stdout)
        self.assertEqual(self.snapshot(), before, result.stdout + result.stderr)
        self.assertFalse(self.tx.exists())

    def test_conflicts_preserved_across_upgrade_and_both_uninstall_paths(self):
        for shell in SHELLS:
            for standalone in (False, True):
                with self.subTest(shell=shell, standalone=standalone):
                    for _ in range(2):
                        r = self.run_script(shell)
                        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
                        self.assertFalse(self.tx.exists())
                        for name in CONFLICTS:
                            self.assertFalse((self.jars / name).exists())
                            self.assertEqual((self.jars / (name + '.carplay-stock')).read_text(), 'original:' + name)
                    r = self.run_script(shell, 'uninstall', source=UNINSTALL if standalone else INSTALL)
                    self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
                    for name in CONFLICTS:
                        self.assertEqual((self.jars / name).read_text(), 'original:' + name)
                    self.assertEqual((self.config / 'smartphone_integrator.json').read_text(), SI)
                    self.assertEqual((self.config / 'dio_manager.json').read_text(), DIO)

    def test_stage_activation_and_configuration_failures_rollback(self):
        for shell in SHELLS:
            for fault in ('stage', 'activate', 'si', 'dio'):
                with self.subTest(shell=shell, fault=fault):
                    (self.config / 'smartphone_integrator.json').write_text(
                        SI.replace('"carplay": {', '"carplay": { "unsupported": true },') if fault == 'si' else SI)
                    (self.config / 'dio_manager.json').write_text(
                        DIO + DIO if fault == 'dio' else DIO)
                    self.assert_failed_and_restored(shell, fault if fault in ('stage', 'activate') else None)

    def test_killed_update_requires_explicit_recovery(self):
        for shell in SHELLS:
            with self.subTest(shell=shell):
                before = self.snapshot()
                r = self.run_script(shell, fault='kill')
                self.assertNotEqual(r.returncode, 0)
                self.assertTrue((self.tx / 'armed').exists())
                self.assertNotEqual(self.snapshot(), before)
                r = self.run_script(shell)
                self.assertNotEqual(r.returncode, 0)
                self.assertIn('recover first', r.stdout)
                r = self.run_script(shell, 'recover')
                self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
                self.assertEqual(self.snapshot(), before)

    def test_terminated_update_rolls_back(self):
        for shell in SHELLS:
            with self.subTest(shell=shell):
                self.assert_failed_and_restored(shell, 'term')

    def test_failed_rollback_retains_journal_for_recovery(self):
        before = self.snapshot()
        r = self.run_script('/bin/sh', fault='rollback')
        self.assertNotEqual(r.returncode, 0)
        self.assertTrue((self.tx / 'journal').exists())
        r = self.run_script('/bin/sh', 'recover')
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertEqual(self.snapshot(), before)

    def test_committed_update_recovery_does_not_undo_success(self):
        r = self.run_script('/bin/sh', fault='committed-kill')
        self.assertNotEqual(r.returncode, 0)
        self.assertTrue((self.tx / 'committed').exists())
        installed = {p.name: p.read_bytes() for p in self.hooks.iterdir() if p.is_file()}
        r = self.run_script('/bin/sh', 'recover')
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertFalse(self.tx.exists())
        self.assertEqual({p.name: p.read_bytes() for p in self.hooks.iterdir() if p.is_file()}, installed)

    def test_live_installer_recovery_is_refused(self):
        self.tx.mkdir()
        (self.tx / 'pid').write_text(str(os.getpid()))
        r = self.run_script('/bin/sh', 'recover')
        self.assertNotEqual(r.returncode, 0)
        self.assertTrue(self.tx.exists())

    def test_truncated_journal_recovery_is_refused_without_changes(self):
        r = self.run_script('/bin/sh', fault='kill')
        self.assertNotEqual(r.returncode, 0)
        (self.tx / 'journal').write_text('')
        before = self.snapshot()
        r = self.run_script('/bin/sh', 'recover')
        self.assertNotEqual(r.returncode, 0)
        self.assertIn('incomplete recovery journal', r.stdout)
        self.assertEqual(self.snapshot(), before)

    def test_conflict_restore_collision_preserves_both_files(self):
        r = self.run_script('/bin/sh')
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        conflict = self.jars / CONFLICTS[0]
        conflict.write_text('independently installed')
        before = self.snapshot()
        for source in (INSTALL, UNINSTALL):
            r = self.run_script('/bin/sh', 'uninstall', source=source)
            self.assertNotEqual(r.returncode, 0)
            self.assertEqual(self.snapshot(), before)

    def test_symlink_destination_is_rejected_without_truncation(self):
        victim = self.unit / 'victim'
        victim.write_text('keep')
        path = self.hooks / ASSETS[0]
        path.unlink()
        path.symlink_to(victim)
        r = self.run_script('/bin/sh')
        self.assertNotEqual(r.returncode, 0)
        self.assertEqual(victim.read_text(), 'keep')
        self.assertTrue(path.is_symlink())
        self.assertFalse(self.tx.exists())


if __name__ == '__main__':
    unittest.main(verbosity=2)
