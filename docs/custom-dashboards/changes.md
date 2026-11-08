# Change ledger

## P2-C05 — shared settings contract (2026-11-08)

User: "start C05 settings contract". Added `DashboardSettings.java`, typed
`DashboardConfig.parse/load` (enabled() unchanged in meaning), shared vectors
`tests/fixtures/dashboard_config_vectors.txt`, `tests/dashboard_config_vectors_test.c`,
vector + typed checks in `DashboardConfigTest`, Makefile/script wiring,
[settings-contract.md](settings-contract.md), config.md pointer, plan/status/HANDOFF/AGENTS.
Format unchanged. Host tests pass; mutants caught (one equivalent mutant noted).

## P2-C04 — signal acceptance and eligibility (2026-11-08)

User: "start C04". Added `dashboard/signal_policy.{h,c}`, runtime integration
(`db_runtime_set_signal_policy`, default PRODUCTION + compiled register),
`tests/dashboard_signal_policy_test.c`, runtime test updates (explicit VALIDATION
mode; production-default case), Makefile wiring, [signal-acceptance.md](signal-acceptance.md),
completion-plan/status/HANDOFF/AGENTS/runtime docs. Host tests pass; mutants caught.
Merged to master on "merge + update docs"; current-state docs and agent pointers updated.

## FIX-CLAUDE-C05 — review corrections (2026-11-08, published)

User: "Fix all findings". Runtime shutdown now disables intent and cancels
pending retries until explicit enable; acquisition requires all output callbacks.
Tests cover shutdown/restart with fresh records, recovery cancellation and every
incomplete backend combination. Normal and sanitizer suites pass. Reconciled
Phase 1 target/profile/roadmap planning as installed and vehicle-tested; updated
runtime contract and agent handoff. [Resolution](reviews/claude-p2-c05-resolution.md)
records validation and Java prerequisite gap. Committed as `62b18e1` and pushed
to fork master on "Commit and push to master"; remote SHA verified. No vehicle action.

## P2-C05 — runtime controller (2026-11-08)

User: "start C05". Added `dashboard/runtime.{h,c}` (states, backend contract,
data-before-ownership, dirty redraw, bounded backoff, config/profile reconciler),
`tests/dashboard_runtime_test.c`, Makefile wiring, [runtime.md](runtime.md);
completion-plan/status updated. Host tests pass incl. sanitizers; mutation checks caught.
Merged to master on the user's "merge + update docs"; AGENTS, HANDOFF, status,
completion-plan C05 state and agent pointers updated.

## DOC-PHASE1-HANDOVER — Phase 1 handover incorporated (2026-11-08)

User-supplied handover stored unchanged in [docs/phase1-handover/](../phase1-handover/README.md)
(three documents; manifest CSV kept with the user). Phase 1 status corrected to
INSTALLED (upstream 1.4.1, vehicle-tested) across target-profile, completion-plan,
requirements, ROADMAP, PROJECT, PLAN, status, HANDOFF, completion-audit, AGENTS and
agent pointers; map-area surface constraint and licensing decision recorded.
Documentation only.

## 2026-11-08 — independent completion audit corrections

See [completion-audit.md](completion-audit.md) for finding disposition, exact
validation and remaining vehicle gates. Changes: installer staging/journal/recovery,
conflicting-JAR preservation and error propagation; configuration/setup/uninstall
safeguards; missing-widget omission/no-data suppression; native regular-file
loaders; unique Java staging and independent restart generations; fault-injection,
omission/FIFO/symlink/lifecycle regressions; dashboard and maintenance CI steps.
No decoder policy, vehicle routing, proprietary assets or Phases 3–5 changes.

## P2-FINAL resume — remaining regressions and shutdown correction (2026-11-07)

Resumed b180e36 from Claude's checkpoint. User subsequently requested completing
remaining work and committing/pushing to master. Available host work is complete;
vehicle rendering remains blocked by the original firmware/safe-surface gate.

| Changed path | Change and purpose |
| --- | --- |
| `java_patch/com/luka/carplay/dashboard/DashboardModule.java` | Always remove the previously published record at stop; stage bulk writes outside the lock, commit generation-check/rename under the stop lock; clean per-generation temp files in their owner. Fixes the reproduced blocked-second-write case left by the earlier R2 correction. |
| `tests/DashboardModuleTest.java` | First publish successfully, block the next write, require immediate clearing on stop, restart while old I/O is pending, verify new session remains. Observes the publication gate instead of a fixed scheduling sleep. RED before runtime correction, GREEN after. |
| `tests/upstream_v141_test.py` | Pins original dashboard/roadmap preservation to the actual f5bd2e7 maintenance commit and checks its ancestry. Current functional tests cover subsequent features; exact release blobs, decoder policy and real installer fixtures remain unchanged. |
| [validation-final.md](validation-final.md) | Complete current commands, passes, reproduced failures, unavailable checks, performance, logs and step-9 prerequisites; preserves Claude's stage history. |
| [vehicle-provider.md](vehicle-provider.md), [review resolution](reviews/claude-312215a-resolution.md), [review index](reviews/README.md) | Update lifecycle contract, residual R2 evidence, current finding states and fulfilled final-test queue. Original pinned review remains unchanged. |
| [PROJECT.md](../../PROJECT.md), [ROADMAP.md](../ROADMAP.md), [status.md](status.md), [PLAN.md](PLAN.md), [HANDOFF.md](HANDOFF.md) | Distinguish completed available host work from blocked vehicle work and record master publication authorization. |
| [module.md](module.md), [rendering.md](rendering.md), [architecture.md](architecture.md), [data-providers.md](data-providers.md), [package.md](package.md), [preview index](../../dashboard/previews/README.md) | Reconcile final-stage testing, implemented provider/manager and all four restored previews; retain historical package evidence. No SVG regenerated during resume. |
| [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), [GEMINI.md](../../GEMINI.md), [Copilot instructions](../../.github/copilot-instructions.md) | Point agents to current validation and publication scope; retain safety/roadmap boundaries. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Resume, RED/GREEN evidence, test outcomes, boundaries and publication checkpoint. |
| [changes.md](changes.md) | Per-file record of this increment. |

No native renderer/hook, installer, deploy, firmware or Android code changed.
No package installation, vehicle access or release. Host tests do not establish
vehicle compatibility. Undo this increment by a reviewed revert, preserving
later work; never reset/force-push master. No on-car rollback is needed because
nothing was installed. Keep dashboard disabled until separate vehicle gates pass.

## FIX-REVIEW-312215A — review fixes (2026-11-07)

User: "check this audit ... implement all fixes". Branch `fix/review-claude-312215a`.
Manual review only; no builds/tests/test code (user instruction). Details:
[resolution record](reviews/claude-312215a-resolution.md).

| Changed path | Change and purpose |
| --- | --- |
| `java_patch/com/luka/carplay/dashboard/DashboardModule.java` | R1 write time after reads; R2 generations, exclusive publication, post-rename cleanup, per-generation temp files; writer session id at bytes 60–63. |
| `java_patch/com/luka/carplay/core/VehicleDataModule.java` | Android Auto dashboard mode binds only DSICarKombi, no sender thread. |
| [dashboard/vehicle_provider.c](../../dashboard/vehicle_provider.c), [dashboard/dashboard.h](../../dashboard/dashboard.h) | R3 clock after read; zero session/sequence rejected; stateful liveness (new session, backwards sequence, 2 s timeout). |
| [PROJECT.md](../../PROJECT.md), [data-providers.md](data-providers.md), [architecture.md](architecture.md), [package.md](package.md), [module.md](module.md), [rendering.md](rendering.md), [ROADMAP.md](../ROADMAP.md), [vehicle-provider.md](vehicle-provider.md) | R5 reconciliation with current source; older claims pinned to their revision. |
| [reviews/claude-312215a-resolution.md](reviews/claude-312215a-resolution.md), [reviews/README.md](reviews/README.md) | New resolution record and index link; report unchanged. |
| [AGENTS.md](../../AGENTS.md), [status.md](status.md), [HANDOFF.md](HANDOFF.md), [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Current task, events and checkpoint. |

## PUBLISH-REVIEW — authorized report publication (2026-11-07)

User requested "Push and commit". Publish the report, pickup guide, agent entry
points, status/plan/handoff/change ledger and audit on review/claude-phase2 to
the fork. Updated AGENTS/review publication notes and current audit to supersede
the older local-only state. No implementation changes, tests, master merge, release
or vehicle actions; publication evidence follows in the audit.

## DOC-REVIEW-HANDOFF — AI pickup instructions (2026-11-07)

User requested a Markdown report and instructions for AI agents to pick it up.
The report already existed; its five findings remain unchanged and unresolved.

| Changed path | Change and purpose |
| --- | --- |
| [reviews/claude-312215a.md](reviews/claude-312215a.md) | Adds links to the agent pickup guide/checkpoint and explicitly marks findings open. |
| [reviews/README.md](reviews/README.md) | New: resumption procedure, ordered R1–R5 queue, acceptance evidence, separate implementation/verification states, no-test policy, safety boundaries and copyable next-agent prompt. |
| [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), [GEMINI.md](../../GEMINI.md), [Copilot instructions](../../.github/copilot-instructions.md) | Makes the review and pickup instructions discoverable; marks the old P2-08 task historical. |
| [HANDOFF.md](HANDOFF.md) | Adds the latest review handoff ahead of older feature milestones. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Records documentation-only scope, local publication state and exact next action. |
| [changes.md](changes.md) | Records this handoff change map. |

No implementation/test code changed or executed. No commit, push, merge, release
or vehicle actions. Earlier local review/report/audit edits are preserved.

## REVIEW-CLAUDE — source review at 312215a (2026-11-07)

User requested review of everything pushed by Claude. Added
[the review report](reviews/claude-312215a.md), status/plan review notices, and
updated audit/CURRENT.md plus append-only audit/LOG.md. Five source/documentation
findings and known validation/vehicle gaps are recorded. No implementation or
test code changed; no builds/tests/automated validation, commit/push or vehicle
actions. These review documents remain local on review/claude-phase2.

This ledger describes concrete changes for reviewers and subsequent agents.
Chronological session evidence remains append-only in [audit/LOG.md](audit/LOG.md).
Current work state is in [audit/CURRENT.md](audit/CURRENT.md).

## P2-08 — VehicleDataProvider (2026-11-07)

User: "merge to master and start step 8". Branch `feature/p2-08-vehicle-provider`.
C compiled; `DashboardConfig.java` compiled standalone; other Java not compilable
here (stock classes absent). Nothing run; no tests written (user instruction).

| Changed path | Change and purpose |
| --- | --- |
| `java_patch/com/luka/carplay/dashboard/DashboardConfig.java` | New: strict v1 config gate (`/mnt/app/carplay_dashboard.cfg`), disabled unless `enabled=1`. |
| `java_patch/com/luka/carplay/dashboard/DashboardModule.java` | New: publishes the DBV1 record every 500 ms while enabled; deletes it when disabled/stopped. |
| `java_patch/com/luka/carplay/core/VehicleDataModule.java` | Runs for any session; binds in AA only if enabled; `CMD_VEHICLE` CarPlay-only; read-only `dashboardVehicle()`. |
| `java_patch/com/luka/carplay/core/CarPlayApp.java` | VehicleDataModule owner -> any; `DashboardModule` added. |
| `java_patch/com/luka/carplay/aa/AaRgState.java`, `AaBridge.java` | Read-only `dashboardNav()` with destination sample time. |
| [dashboard/vehicle_provider.c](../../dashboard/vehicle_provider.c) | New: DBV1 reader/validator with clock mapping. |
| [dashboard/dashboard.h](../../dashboard/dashboard.h), [widgets.c](../../dashboard/widgets.c), [Makefile](../../dashboard/Makefile) | `DB_SOURCE_VEHICLE` + UNVERIFIED badge; vehicle provider API; build wiring. |
| [vehicle-provider.md](vehicle-provider.md) | New: data path, signals, time rules, record layout, limitations, tests needed. |
| [requirements.md](requirements.md), [PLAN.md](PLAN.md), [status.md](status.md), [HANDOFF.md](HANDOFF.md), [data-providers.md](data-providers.md), [AGENTS.md](../../AGENTS.md) | Step-8 state. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Events and checkpoint. |

## P2-07 — verified-source widgets only (2026-11-07)

User "yes" to starting PLAN step 7. Branch `feature/p2-07-verified-widgets`.
Compiled only; no tests run or edited (user instruction). Existing tests are now
out of date; list in [widgets-p2-07.md](widgets-p2-07.md).

| Changed path | Change and purpose |
| --- | --- |
| [dashboard/dashboard.h](../../dashboard/dashboard.h) | Removes trip/consumption types, fields and widgets; adds `DB_NAV_DISTANCE`, `DB_NAV_TIME`, `DB_DESTINATION`; documents -1/empty "not supplied". |
| [dashboard/widgets.c](../../dashboard/widgets.c) | Formatters for the split navigation widgets; not-supplied parts render UNAVAILABLE; Compact navigation shows supplied parts. |
| [dashboard/layout.c](../../dashboard/layout.c) | Extended = temperature, range, now playing, distance, time, destination. |
| [dashboard/mock_provider.c](../../dashboard/mock_provider.c) | Drops trip/consumption values; marks mock as tests/development only. |
| `dashboard/previews/extended-{wide,tall}.svg` | Deleted (showed removed widgets); regenerate at final testing stage. |
| [widgets-p2-07.md](widgets-p2-07.md) | New: rationale, widget contract, signals deliberately not shown, out-of-date tests. |
| [requirements.md](requirements.md), [PLAN.md](PLAN.md), [status.md](status.md), [HANDOFF.md](HANDOFF.md), [module.md](module.md), [rendering.md](rendering.md), [architecture.md](architecture.md), [data-providers.md](data-providers.md), [package.md](package.md), [previews README](../../dashboard/previews/README.md) | Reflect the new widget set and step-7 state. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Checkpoint and events. |

## DOC-SCOPE — Phase 2 "Real Vehicle Data Integration" scope (2026-11-07)

The user supplied the authoritative Phase 2 scope and noted the docs should
already reflect it; they did not. Documentation only; no tests or code changes.

| Changed path | Change and purpose |
| --- | --- |
| [requirements.md](requirements.md) | Adds the verbatim scope, a per-requirement gap table (Phase 2 not complete), and open user decisions; marks the earlier MVP definition superseded where it conflicts; Extended trip/consumption superseded by req. 8. |
| [ROADMAP.md](../ROADMAP.md) | Phase 2 renamed and redefined; status "not complete"; exit target is in-car real data. Fixes stale "P2-04 in progress". |
| [PLAN.md](PLAN.md) | Steps 1–5 marked foundation; step 6 broadened; proposed steps 7–10 (omit unverified widgets, VehicleDataProvider, VC renderer, in-car validation). |
| [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), [GEMINI.md](../../GEMINI.md), [Copilot instructions](../../.github/copilot-instructions.md), [PROJECT.md](../../PROJECT.md) | Goal replaced with the scope; Phase 2 not complete; host fallback is a blocked Phase 2, not a completion. Safety boundary unchanged pending user decision. |
| [status.md](status.md), [HANDOFF.md](HANDOFF.md), [package.md](package.md) | Scope-first summaries and next eligible step. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Scope event and checkpoint. |

## P2-05 — reviewable increment package (2026-11-07)

User request: "Start P2-05". Branch `feature/p2-05-package`. Documentation only;
no tests, builds, demos or preview generation (user's no-test instruction).

| Changed path | Change and purpose |
| --- | --- |
| [package.md](package.md) | New reviewer entry point: quick start, artifacts, historical P2-04 outcomes, limitations/integration blocker, change summary vs upstream, rollback, step-5 acceptance. |
| [previews README](../../dashboard/previews/README.md) | States preview verification history and links the package page. |
| [PLAN.md](PLAN.md), [status.md](status.md), [HANDOFF.md](HANDOFF.md), [AGENTS.md](../../AGENTS.md), [PROJECT.md](../../PROJECT.md) | Record P2-05 scope/state and link package.md. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | START and COMPLETE events; checkpoint. |
| [changes.md](changes.md) | This section. |

## P2-04 — final testing stage (2026-11-07)

User request: "Start P2-04, but before make sure you update the documentation,
as instructed". Branch `feature/p2-04-validation`.

Documentation update (first commit, before any test):

| Changed path | Change and purpose |
| --- | --- |
| [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), [GEMINI.md](../../GEMINI.md), [Copilot instructions](../../.github/copilot-instructions.md) | Current task P2-04; P2-02 no-test override marked historical/ended; tests authorized on the host only. |
| [PROJECT.md](../../PROJECT.md), [PLAN.md](PLAN.md), [HANDOFF.md](HANDOFF.md), [status.md](status.md) | P2-04 scope, branch and step-4 state; P2-03 marked complete/merged. |
| [audit/README.md](audit/README.md), [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Scope change recorded; START event and checklist. |
| [module.md](module.md), [rendering.md](rendering.md), [config.md](config.md), [architecture.md](architecture.md), [requirements.md](requirements.md), [data-providers.md](data-providers.md), [ROADMAP.md](../ROADMAP.md), [previews README](../../dashboard/previews/README.md) | Active "do not run tests" statements replaced with the P2-04 testing state. |

Tests and report (second commit):

| Changed path | Change and purpose |
| --- | --- |
| [tests/dashboard_manager_test.c](../../tests/dashboard_manager_test.c) | New C suite for P2-03 config parser/limits/actions/loader and manager gating, clearing and fallback. |
| [tests/dashboard_cli_test.py](../../tests/dashboard_cli_test.py) | New stdlib-only checks for `dashboard_config` and `dashboard_manager_preview` outputs and exit codes. |
| [dashboard/benchmark.c](../../dashboard/benchmark.c) | New bounded host benchmark (fixed iterations, mock provider). |
| [dashboard/Makefile](../../dashboard/Makefile) | Adds the new suites to `test`/`test-sanitize`, a `benchmark` target and cleanup. |
| [validation.md](validation.md) | New P2-04 regression report: commands, results, unavailable checks with causes, benchmark, acceptance. |
| [status.md](status.md), [PLAN.md](PLAN.md), [HANDOFF.md](HANDOFF.md), [config.md](config.md), [module.md](module.md) | Record P2-04 results and the next eligible step. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Checkpoints and COMPLETE event. |

## P2-03 — host configuration and dashboard manager (2026-11-07)

User request: "Start P2-03 on a dedicated feature branch". Branch
`feature/p2-03-config-manager`. Strict C99 compile only; no tests executed
(final-stage deferral). No GEM, upstream runtime, Android, release or vehicle change.

| Changed path | Change and purpose |
| --- | --- |
| [dashboard/dashboard.h](../../dashboard/dashboard.h) | Adds `db_config` (v1), parse/load/write/action API, `db_manager` state, status enum and manager API. Existing P2-01/P2-02 declarations unchanged. |
| [dashboard/config.c](../../dashboard/config.c) | New strict bounded parser (first key `version=1`, no whitespace/unknown/duplicate keys, 512-byte/64-byte-line caps), file loader distinguishing missing vs invalid, canonical writer, enable/disable/select-layout/restore-default actions. Every failure yields disabled Compact. |
| [dashboard/manager.c](../../dashboard/manager.c) | New single-provider manager: config gate → one `db_provider` read → layout build. Clears snapshot and frame on invalid config, disabled state, provider failure or geometry rejection, and on every config action. Disabled/invalid states never read the provider. |
| [dashboard/dashboard_config.c](../../dashboard/dashboard_config.c) | New host CLI applying one action and printing canonical config to stdout; layout actions keep enablement. |
| [dashboard/manager_preview.c](../../dashboard/manager_preview.c) | New host CLI running the manager with an explicitly chosen MOCK provider; SVG only when READY, else empty stdout and exit 3. |
| [dashboard/Makefile](../../dashboard/Makefile) | Adds config.c/manager.c to the library and test sources, the two tools to `all`, and their cleanup. |
| [config.md](config.md) | New P2-03 contract: format, actions, manager order, CLIs, deferred acceptance checks and rollback. |
| [PLAN.md](PLAN.md), [status.md](status.md), [HANDOFF.md](HANDOFF.md), [module.md](module.md), [architecture.md](architecture.md), [ROADMAP.md](../ROADMAP.md), [AGENTS.md](../../AGENTS.md) | Record P2-03 as implemented-but-untested on its branch and update stale "manager planned" statements. |
| [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | START, design-fix and handoff events; current checkpoint. |
| [changes.md](changes.md) | This section. |

## PUBLISH-MASTER — approved consolidation (2026-11-07)

User authorized pushing everything to the fork's main/master. Master fast-forwarded
from b127988 to f5bd2e7 and was pushed with a verified remote SHA. All dashboard,
roadmap, agent and maintenance work is included; prior branches remain. No merge
conflicts, source rewrites, new features, tests, release or vehicle execution.

| Changed path | Change and purpose |
| --- | --- |
| [PROJECT.md](../../PROJECT.md) | Uses master clone instructions and describes consolidated project contents. |
| [AGENTS.md](../../AGENTS.md), [CLAUDE.md](../../CLAUDE.md), [GEMINI.md](../../GEMINI.md), [Copilot instructions](../../.github/copilot-instructions.md) | Replaces stale local-only scope with explicit publication approval and retained safety/phase boundaries. |
| [status.md](status.md), [PLAN.md](PLAN.md), [HANDOFF.md](HANDOFF.md) | Records master publication, preserves prior test evidence, awaits a new task before P2-03. |
| [maintenance report](../maintenance/upstream-v1.4.1.md) | Adds later publication context and history-preserving rollback guidance. |
| [audit/README.md](audit/README.md), [audit/CURRENT.md](audit/CURRENT.md), [audit/LOG.md](audit/LOG.md) | Records authorization, fast-forward and verified source SHA, plus checkpoint publication handoff. |
| [changes.md](changes.md) | Records this documentation-only publication change map. |

## MAINT-141 — existing v1.4.1 integration verification (2026-11-07)

The exact release already existed in project ancestry. No upstream runtime,
dashboard code/tests/previews, interface, provider design or five-phase roadmap
changed. This increment was originally local-only; the later approved master
publication is recorded above. No release or vehicle actions.

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
