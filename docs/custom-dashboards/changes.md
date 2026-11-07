# Change ledger

This ledger describes concrete changes for reviewers and subsequent agents.
Chronological session evidence remains append-only in [audit/LOG.md](audit/LOG.md).
Current work state is in [audit/CURRENT.md](audit/CURRENT.md).

## P2-02 — layouts and host SVG (2026-11-07)

User authorized step 2 and committing/pushing. During work, the user instructed:
"Sa nu executi teste momentan, testele sunt pentru final, mergem inainte fara
teste. Doar sa documentezi fiecare schimbare." Further tests and automated
validation are deferred until the final stage. Prior results remain in the audit;
no final-validation or successful visual-inspection claim is made.

| Changed path | Change and purpose |
| --- | --- |
| [dashboard/dashboard.h](../../dashboard/dashboard.h) | Turns reserved frame types into an exposed host layout/export contract: layout enum, builder, whole-frame validator and SVG writer. Documents bounds, failure clearing and stream ownership. QNX guard remains. |
| [dashboard/layout.c](../../dashboard/layout.c) | Adds Compact 2x2 and Extended 3x2 grids, theme, badges, explicit viewport handling and bounded text elision. Uses P2-01 widget formatting; invalid input clears previous frames. No runtime or vehicle hook. |
| [dashboard/svg.c](../../dashboard/svg.c) | Adds strict geometry/text/command validation and standalone SVG export with XML escaping, per-text clips and outer viewport clip. Rejects invalid frames before output and reports observed stream errors. |
| [dashboard/preview.c](../../dashboard/preview.c) | Adds mock-only CLI requiring explicit compact/extended and wide/tall arguments. Exports to stdout; no persistent settings or display window. |
| [dashboard/Makefile](../../dashboard/Makefile) | Adds layout/export objects, preview binary, preview generation and C/Python test targets. Sanitizer target covers both C suites. New tests are prepared for final-stage use, not automatic agent execution under the current instruction. |
| [tests/dashboard_render_test.c](../../tests/dashboard_render_test.c) | Adds content/bounds/crop/clearing/placeholder/elision/malformed-frame/XML/I/O test cases. Written and first executed before test deferral; retained for final validation. |
| [tests/dashboard_svg_test.py](../../tests/dashboard_svg_test.py) | Adds independent XML/CLI/determinism/provenance/clip/artifact checks using Python stdlib. Written and first executed before test deferral. |
| [dashboard/previews/compact-wide.svg](../../dashboard/previews/compact-wide.svg) | Saved 800x252 Compact mock preview, generated before test deferral. |
| [dashboard/previews/compact-tall.svg](../../dashboard/previews/compact-tall.svg) | Saved 640x360 Compact mock preview, generated before test deferral. |
| [dashboard/previews/extended-wide.svg](../../dashboard/previews/extended-wide.svg) | Saved 800x252 Extended mock preview, generated before test deferral. |
| [dashboard/previews/extended-tall.svg](../../dashboard/previews/extended-tall.svg) | Saved 640x360 Extended mock preview, generated before test deferral. |
| [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), [GEMINI.md](../../GEMINI.md), [Copilot instructions](../../.github/copilot-instructions.md) | Update active scope to P2-02 and require all agents to honor the user's final-stage-only testing policy. |
| [PROJECT.md](../../PROJECT.md) | Updates implemented scope, commands and navigation links to rendering/changes/previews. |
| [PLAN.md](PLAN.md), [architecture.md](architecture.md), [requirements.md](requirements.md), [analysis.md](analysis.md) | Separate implemented host graphics from future config/vehicle work and distinguish earlier test evidence from deferred final validation. |
| [module.md](module.md), [rendering.md](rendering.md) | Preserve the P2-01 contract description while documenting current layout/export API, typography, CLI and limitations. |
| [HANDOFF.md](HANDOFF.md), [status.md](status.md) | Record implemented step 2, no-test override, preview limitations and next eligible step 3. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Record authorization, changes, prior results/failures, user steering, unfinished validation and publication checkpoints. |
| [audit/README.md](audit/README.md) | Requires resumed agents to honor test deferral instead of running checks to populate audit fields. |
| [dashboard/previews/README.md](../../dashboard/previews/README.md) | Gives direct preview links, generation instructions and explicit host/mock/final-review caveats. |

Inherited renderers, hooks, Java, vehicle data, stock files and deployment scripts
are unchanged. No dependency was installed. Temporary screenshot attempts used
local tools and failed; screenshots are not supplied or represented as passed.

## Prior increment

P2-01 implementation is commit `0b256c07d736a3f47062c9f4afa751dd5b4a1e20`;
its API and results are documented in [module.md](module.md) and [status.md](status.md).
This ledger starts with P2-02; earlier decisions remain in Git and the audit log.
