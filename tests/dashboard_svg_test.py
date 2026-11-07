#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Independent XML/CLI checks for exported SVG; no third-party dependencies."""
from pathlib import Path
import subprocess
import sys
import xml.etree.ElementTree as ET

SVG = "{http://www.w3.org/2000/svg}"


def check_svg(data, widgets, size):
    root = ET.fromstring(data)
    assert root.tag == SVG + "svg"
    assert (int(root.attrib["width"]), int(root.attrib["height"])) == size
    assert root.attrib["viewBox"] == f"0 0 {size[0]} {size[1]}"
    assert not list(root.iter(SVG + "script"))
    texts = list(root.iter(SVG + "text"))
    assert sum(t.text == "MOCK" for t in texts) == widgets
    assert any(t.text == "HOST PREVIEW - NO VEHICLE OUTPUT" for t in texts)
    clips = {e.attrib["id"]: e for e in root.iter(SVG + "clipPath")}
    for t in texts:
        clip = clips[t.attrib["clip-path"][5:-1]]
        box = clip.find(SVG + "rect")
        x, y, w, h = [int(box.attrib[n]) for n in ("x", "y", "width", "height")]
        assert 0 <= x < x+w <= size[0] and 0 <= y < y+h <= size[1]
        assert int(t.attrib["x"]) == x
        assert y <= int(t.attrib["y"]) < y+h
    return root


def main():
    binary, fixture = map(lambda s: Path(s).resolve(), sys.argv[1:])
    for layout, count in (("compact", 4), ("extended", 6)):
        for profile, size in (("wide", (800, 252)), ("tall", (640, 360))):
            args = [str(binary), layout, profile]
            data = subprocess.check_output(args)
            assert data == subprocess.check_output(args), "nondeterministic SVG"
            root = check_svg(data, count, size)
            assert any(t.text == "42.5 km / 38 min" for t in root.iter(SVG + "text"))
            saved = binary.parents[2] / "dashboard" / "previews" / f"{layout}-{profile}.svg"
            if saved.exists():
                assert saved.read_bytes() == data, f"stale saved preview: {saved}"
    for args in ([], ["compact"], ["bogus", "wide"], ["compact", "bogus"], ["compact", "wide", "extra"]):
        result = subprocess.run([str(binary), *args], capture_output=True)
        assert result.returncode == 2 and not result.stdout and b"Usage:" in result.stderr
    escaped = fixture.read_bytes()
    for entity in (b"&lt;", b"&gt;", b"&amp;", b"&quot;", b"&apos;"):
        assert entity in escaped
    root = ET.fromstring(escaped)
    assert any(t.text == "<script>&\"' >" for t in root.iter(SVG + "text"))
    assert not list(root.iter(SVG + "script"))
    print("dashboard_svg_test: 4 deterministic SVGs, XML/clips/provenance, CLI failures and escaped text round trip PASS")


if __name__ == "__main__":
    main()
