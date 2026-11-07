# Host dashboard previews

All displayed values are MOCK. These are standalone SVG artifacts, not vehicle
screenshots or verified Virtual Cockpit layouts. No safe in-car dashboard area
has been established. P2-04 confirmed these files match CLI output; visual and
readability review on a real cluster has not been done. Package overview:
[package.md](../../docs/custom-dashboards/package.md).

| Layout | 800x252 | 640x360 |
| --- | --- | --- |
| Compact | [Wide](compact-wide.svg) | [Tall](compact-tall.svg) |
| Extended | [Wide](extended-wide.svg) | [Tall](extended-tall.svg) |

![Compact](compact-wide.svg)

The old Extended previews showed trip/consumption widgets removed in P2-07.
P2-FINAL restored both artifacts using the new destination distance/time/name
widgets. All four SVGs match current CLI output; see
[final validation](../../docs/custom-dashboards/validation-final.md).

Generation command from the repository root: `make -C dashboard previews`.
Manager-gated output remains available through `dashboard_manager_preview`;
it is not saved. The previews remain MOCK and do not prove an on-car layout.
See [rendering.md](../../docs/custom-dashboards/rendering.md) for the API, design
limits and commands, and [changes.md](../../docs/custom-dashboards/changes.md)
for the per-file change record. No screenshot or font-fit claim is made.
