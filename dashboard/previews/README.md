# Host dashboard previews

All displayed values are MOCK. These are standalone SVG artifacts, not vehicle
screenshots or verified Virtual Cockpit layouts. No safe in-car dashboard area
has been established. P2-04 confirmed these files match CLI output; visual and
readability review on a real cluster has not been done. Package overview:
[package.md](../../docs/custom-dashboards/package.md).

| Layout | 800x252 | 640x360 |
| --- | --- | --- |
| Compact | [Wide](compact-wide.svg) | [Tall](compact-tall.svg) |
| Extended | removed in P2-07 | removed in P2-07 |

![Compact](compact-wide.svg)

The Extended previews showed trip distance and average consumption, which P2-07
removed (no verified source). They will be regenerated at the final testing
stage; see [widgets-p2-07.md](../../docs/custom-dashboards/widgets-p2-07.md).

Generation command from the repository root: `make -C dashboard previews`.
The saved artifacts were generated in P2-02 and verified byte-identical to CLI
output in P2-04. No dashboard code changed afterwards, so P2-05 did not
regenerate them. A manager-gated preview can be produced on demand with
`dashboard_manager_preview` (see config.md); it is not saved.
See [rendering.md](../../docs/custom-dashboards/rendering.md) for the API, design
limits and commands, and [changes.md](../../docs/custom-dashboards/changes.md)
for the per-file change record. No screenshot or font-fit claim is made.
