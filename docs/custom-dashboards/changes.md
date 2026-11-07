# Change ledger

This ledger describes concrete changes for reviewers and subsequent agents.
Chronological session evidence remains append-only in [audit/LOG.md](audit/LOG.md).
Current work state is in [audit/CURRENT.md](audit/CURRENT.md).

## MAINT-141 — existing v1.4.1 integration verification (2026-11-07)

The exact release already existed in project ancestry. No upstream runtime,
dashboard code/tests/previews, interface, provider design or five-phase roadmap
changed. This increment stays local; no push/merge/release or vehicle actions.

| Changed path | Change and purpose |
| --- | --- |
| [tests/upstream_v141_test.py](../../tests/upstream_v141_test.py) | Adds exact 20-file release/ancestry and custom-source preservation checks, extracted renderer policy regression, and real old/current program + menu upgrade fixtures covering delayed preference OFF/ON, other settings, idempotency, original backups and uninstall. Temp host paths/inert payloads only. |
| [docs/maintenance/upstream-v1.4.1.md](../maintenance/upstream-v1.4.1.md) | Full three-commit/20-file mapping, no-conflict/already-integrated evidence, reproducible tests and actual outcomes, missing toolchains/firmware, risks and A4 B9 pre-install/future hardware test plan. |
| [PROJECT.md](../../PROJECT.md) | Links the local maintenance report and explicit no-publication scope. |
| [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), [GEMINI.md](../../GEMINI.md), [Copilot instructions](../../.github/copilot-instructions.md) | Records maintenance-specific test authorization and prohibits automatic push/merge/release. |
| [status.md](status.md), [PLAN.md](PLAN.md), [HANDOFF.md](HANDOFF.md) | Separates maintenance verification from roadmap development; keeps earlier feature checkpoints as history, adds current results and links. |
| [audit/README.md](audit/README.md), [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Preserves authorization, fixture correction/test outcomes, source proof and local review handoff. |
| [changes.md](changes.md) | Records this per-file maintenance change map. |

## DOC-ROADMAP — five-phase context and shared providers (2026-11-07)

Documentation-only update requested by the user. No application, adapter,
communication, runtime or preview code changed. No tests or automated checks run;
the final-stage-only policy remains active.

| Changed path | Change and purpose |
| --- | --- |
| [docs/ROADMAP.md](../ROADMAP.md) | Complete five-phase project context, current status, deliverables, boundaries and common provider flow; distinguishes P2-03 configuration from roadmap Phase 3 app work. |
| [data-providers.md](data-providers.md) | Maps the existing callback/snapshot contract to Phase 2 VehicleDataProvider and future AndroidDataProvider; documents shared manager/widgets, identity/quality separation, normalization, clock/session handling and source selection. Clearly marks unimplemented adapters/manager/extensions. |
| [architecture.md](architecture.md) | Updates target wiring to show vehicle/mock/Android through the same interface and manager; separates implemented API from planned source resolution. |
| [PLAN.md](PLAN.md) | Links the complete roadmap, keeps phase implementation boundaries and requires P2-03 manager inputs to remain provider independent. No new implementation started. |
| [requirements.md](requirements.md) | Adds common provider evolution as an acceptance target and records standalone-app/communication/dashboard phases separately. |
| [module.md](module.md) | Links future adapters to the existing in-process contract and identifies missing provider identity/session metadata. |
| [PROJECT.md](../../PROJECT.md) | Adds the complete long-term context and links the roadmap/provider design from the project entry point. |
| [AGENTS.md](../../AGENTS.md) | Makes roadmap and provider design mandatory agent reading; permits future-phase planning while continuing to prohibit premature Android implementation and vehicle actions. |
| [CLAUDE.md](../../CLAUDE.md), [GEMINI.md](../../GEMINI.md), [Copilot instructions](../../.github/copilot-instructions.md) | Point all supported agent entry files at the same five-phase context and shared provider architecture. |
| [HANDOFF.md](HANDOFF.md) | Adds roadmap/provider reading to the resume prompt and highlights phase/subtask numbering. |
| [status.md](status.md) | Records documentation scope, unimplemented live adapters and no test execution. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Record user architecture requirement, source evidence, documentation progress and publication handoff. |
| [changes.md](changes.md) | Records this per-file change map for future sessions. |

The Phase 2 VehicleDataProvider role is required by design, not relabeled mock
data. Current code still ships only db_mock_read. Android communication remains
unimplemented, and provider identity/resolver/manager work is explicitly future.

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
