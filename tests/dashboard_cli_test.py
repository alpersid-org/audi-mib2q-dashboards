#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""P2-04 CLI checks for dashboard_config and dashboard_manager_preview (stdlib only)."""
from pathlib import Path
import os
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

SVG = "{http://www.w3.org/2000/svg}"
HEADER = "# Custom dashboards host configuration (no vehicle output)\n"


def canon(enabled, layout):
    return (HEADER + f"version=1\nenabled={enabled}\nlayout={layout}\n").encode()


def run(*args):
    return subprocess.run([str(a) for a in args], capture_output=True, timeout=5)


def main():
    config_bin, preview_bin = map(lambda s: Path(s).resolve(), sys.argv[1:])
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        missing = tmp / "missing.cfg"
        bad = tmp / "bad.cfg"
        bad.write_bytes(b"version=1\nenabled=yes\n")
        on_ext = tmp / "on.cfg"
        on_ext.write_bytes(canon(1, "extended"))

        # dashboard_config: default, actions, layout keeps enablement.
        r = run(config_bin, "show")
        assert r.returncode == 0 and r.stdout == canon(0, "compact") and not r.stderr
        r = run(config_bin, "show", missing)
        assert r.returncode == 0 and r.stdout == canon(0, "compact") and b"missing" in r.stderr
        r = run(config_bin, "show", bad)
        assert r.returncode == 1 and r.stdout == canon(0, "compact") and b"invalid" in r.stderr
        for action, src, want in (
            ("enable", missing, canon(1, "compact")),
            ("disable", on_ext, canon(0, "extended")),
            ("compact", on_ext, canon(1, "compact")),
            ("extended", missing, canon(0, "extended")),
            ("restore-default", on_ext, canon(0, "compact")),
            ("enable", bad, canon(1, "compact")),
        ):
            r = run(config_bin, action, src)
            assert r.returncode == 0 and r.stdout == want, (action, src, r)
        for args in ([], ["bogus"], ["show", "a", "b"]):
            r = run(config_bin, *args)
            assert r.returncode == 2 and not r.stdout and b"Usage:" in r.stderr

        # Configuration must reject directories, symlinks and a FIFO without
        # blocking. A timeout turns a regression into a bounded failure.
        directory = tmp / "directory.cfg"
        directory.mkdir()
        fifo = tmp / "fifo.cfg"
        os.mkfifo(fifo)
        link = tmp / "link.cfg"
        link.symlink_to(on_ext)
        for path in (directory, fifo, link):
            r = run(config_bin, "show", path)
            assert r.returncode == 1 and r.stdout == canon(0, "compact")
            r = run(preview_bin, path, "wide")
            assert r.returncode == 3 and not r.stdout

        # Output of the tool is valid input to itself (round trip).
        step = tmp / "step.cfg"
        step.write_bytes(run(config_bin, "extended").stdout)
        step.write_bytes(run(config_bin, "enable", step).stdout)
        assert step.read_bytes() == canon(1, "extended")

        # dashboard_manager_preview: suppressed cases write nothing, exit 3.
        off = tmp / "off.cfg"
        off.write_bytes(canon(0, "extended"))
        for cfg, reason in ((missing, b"disabled"), (bad, b"invalid-config"),
                            (off, b"disabled")):
            r = run(preview_bin, cfg, "wide")
            assert r.returncode == 3 and not r.stdout and reason in r.stderr, (cfg, r)
        for profile, size, compact_cfg in (("wide", (800, 252), canon(1, "compact")),
                                           ("tall", (640, 360), canon(1, "compact"))):
            on_cmp = tmp / "on_cmp.cfg"
            on_cmp.write_bytes(compact_cfg)
            for cfg, badges in ((on_cmp, 4), (on_ext, 6)):
                r = run(preview_bin, cfg, profile)
                assert r.returncode == 0 and not r.stderr
                root = ET.fromstring(r.stdout)
                assert (int(root.attrib["width"]), int(root.attrib["height"])) == size
                texts = [t.text for t in root.iter(SVG + "text")]
                assert texts.count("MOCK") == badges
                assert "HOST PREVIEW - NO VEHICLE OUTPUT" in texts
                assert r.stdout == run(preview_bin, cfg, profile).stdout
        for args in ([], [on_ext], [on_ext, "bogus"], [on_ext, "wide", "x"]):
            r = run(preview_bin, *args)
            assert r.returncode == 2 and not r.stdout and b"Usage:" in r.stderr
    print("dashboard_cli_test: config actions/exit codes/round trip, manager preview suppression (exit 3, empty stdout) and READY SVGs PASS")


if __name__ == "__main__":
    main()
