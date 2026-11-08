# Project status

**P2-C05 settings contract (2026-11-08, branch `feature/p2-c05-settings`):** [settings-contract.md](settings-contract.md) — typed Java `DashboardSettings`, 48 shared conformance vectors run by both C and Java; host tests pass. C05 complete on the host.

**P2-C04 (2026-11-08, host part merged to master):** signal acceptance register [signal-acceptance.md](signal-acceptance.md) and per-signal eligibility (`signal_policy.{h,c}`); production shows ACCEPTED signals only and none is accepted yet; items 2–4 wait for C02 vehicle evidence.

**P2-C05 review corrections (published, 2026-11-08, `62b18e1`):** [resolution](reviews/claude-p2-c05-resolution.md)
closes all three findings: shutdown stays disabled until explicit enable,
incomplete backends cannot acquire output, and Phase 1 instructions accept the
installed/vehicle-tested upstream baseline. New regressions, normal C/XML/CLI and
ASan/UBSan suites PASS. Java unavailable (no javac). Fast-forwarded and pushed to
fork master on the user's request; remote source SHA verified.

**P2-C05 (2026-11-08, merged to master):** host runtime controller [runtime.md](runtime.md) — states, data-before-ownership, dirty redraw, bounded retry, config/profile reconciler, backend contract; host tests and sanitizers pass. Remaining in C05: shared Java/C settings contract. No native backend; nothing on the vehicle.

**Phase 1 handover (2026-11-08, supersedes "NOT INSTALLED"):** [Phase 1 handover](../phase1-handover/README.md) shows the
upstream mib2q-carplay **1.4.1 release** installed via M.I.B. V3.8.0 and vehicle-
tested: AA cluster map wired and wireless (AAWireless Gen1), Classic and wide view,
OEM functions intact. The unit's `gal` matches upstream profile P5152 (pinned).
Surface constraint (INFERRED, high confidence): dashboards can occupy only the VC
map area; only displayables 98/99 are proven. No post-reboot log set exists yet.

**Target update:** [target-profile.md](target-profile.md) records user-supplied
P5092/MU1329, media driver DEV_MMX2QC_AU_ER_G22_201PROD-1, VC Classic/Extended Map,
Galaxy S23 Ultra and original AAWireless cs307-e2-prod / 3.5.0. MMI Android Auto
works; **Phase 1 is installed** (upstream 1.4.1, vehicle-tested; Phase 1 handover supersedes the earlier "NOT INSTALLED"). Target identity is known as supplied; actual
stock ABI, enhanced cluster behavior, signal validity and safe geometry remain
open. Prepare/validate the original baseline before hardware dashboard validation;
continue independent host work. Older unconfirmed-installation notes are historical.

**Current completion execution plan:** [completion-plan.md](completion-plan.md),
2026-11-08. C01-C09 map every remaining audit point to dependencies and acceptance;
CI/host preparation can proceed while target/surface evidence is collected.
Phase 2 remains incomplete until actual target integration and vehicle acceptance.

**Current audit corrections (2026-11-08):** [completion-audit.md](completion-audit.md)
records installer recovery, widget omission, feed I/O/restart hardening and CI.
Host checks pass; firmware/safe-surface/stock-linkage/vehicle gates remain open.
Phase 2 is not complete. Earlier stage notes below retain their historical scope.

## P2-FINAL host testing — available checks complete

Dashboard C/XML/CLI, ASan/UBSan, Java feed/Java-to-C, six lifecycle scenarios,
all 39 upstream host checks and all four maintenance methods pass. A remaining
blocked-second-write shutdown defect was reproduced and corrected. Renderer
FIN/RST and PID identity checks pass. Stock Java/DSI/QNX and macOS GL checks remain
unavailable on this host; native script's lane portion passes. Full evidence,
failures before correction and commands: [validation-final.md](validation-final.md).
The user authorized committing/pushing this increment to master. No release or
vehicle action; Phase 2 remains incomplete and step 9 blocked.

The milestone notes below are historical; use the final report for current state.

## Review at 312215a (Codex, 2026-11-07)

Manual review of all Claude commits is recorded in [the review](reviews/claude-312215a.md).
Five findings remain open, especially P2-08 timestamp ordering and shutdown races.
No implementation changed or tests ran. Older status sections below contain stale
statements identified by R5; use the audit for current task state.

## Review fixes (2026-11-07)

Codex reviewed 312215a ([report](reviews/claude-312215a.md)). Fixes for R1, R2,
R3, R5, session invalidation and the AA subscription gap are implemented and
untested on `fix/review-claude-312215a`; R4 waits for the final test request
([resolution](reviews/claude-312215a-resolution.md)). Merged to master.

## Phase 2 scope and state (read first)

The user defined Phase 2 as "Real Vehicle Data Integration": working Compact and
Extended dashboards inside the Virtual Cockpit showing real MMI data, mock only
for tests ([scope and gap table](requirements.md#phase-2-scope--real-vehicle-data-integration-authoritative)).
**Phase 2 is not complete.** Steps P2-01..P2-05 delivered and tested a mock-only
host foundation. Real data binding, VC rendering and in-car validation are
outstanding (PLAN steps 6, 8–10). P2-07 removed the unbacked trip/consumption
widgets and redefined Extended ([widgets-p2-07.md](widgets-p2-07.md)); the
existing tests were repaired in P2-FINAL and the four previews checked. P2-08 adds the real
VehicleDataProvider on `feature/p2-08-vehicle-provider`
([vehicle-provider.md](vehicle-provider.md)): HMI-side publisher plus C reader;
not compiled in full (stock classes absent), not run, unverified on the car. Head-unit work needs the user's
decision on the safety boundary and display surface.

**Final Phase 2 host testing stage (current, P2-FINAL):** the user requested
"Start the final Phase 2 host testing stage. Recheck R1–R5 against the current code, repair the harness and outdated suites, add the missing vehicle-record, Java/C and lifecycle tests, and run applicable host regressions. Document every change and distinguish passed, failed and unavailable checks. Do not deploy, install, flash or execute anything on the vehicle. Keep step 9 blocked pending firmware and safe-surface evidence."
This is the final request the earlier no-test instruction waited for: host tests,
new test code, harness repair, sanitizers, benchmarks and preview regeneration are
authorized on the host. Record exact commands and results as PASSED, FAILED or
UNAVAILABLE (missing prerequisite). No vehicle deployment, installation, flashing
or execution; step 9 stays blocked pending firmware and safe-surface evidence.

**Testing policy after P2-04 (historical, ended by P2-FINAL):** "do not run any test, there will be a ask at the end to implement and run the tests".
Do not run any test, sanitizer, benchmark, demo or automated validation, and do
not write new test code, until the user explicitly asks to implement and run
the tests at the end. Continue documenting every change; record skipped checks
as DEFERRED BY USER. P2-04 results remain historical evidence only.

## Historical work — P2-05 (package the increment)

User request: "Start P2-05". Branch `feature/p2-05-package` from master f7e9fb2.
Added [package.md](package.md): quick start, artifacts, historical P2-04
outcomes, limitations and the VC-surface integration blocker, change summary
vs upstream (48 added paths, no upstream file modified/deleted, by read-only
git) and rollback. Nothing run (user's no-test instruction). Fast-forwarded to
master on the user's "go ahead with the next steps".

## Completed — P2-04 (final testing stage)

User request: "Start P2-04, but before make sure you update the documentation, as instructed". Branch `feature/p2-04-validation` from master
12cb0b5. The P2-02 test deferral ended when the user started P2-04, the final testing
stage it was waiting for. Full report: [validation.md](validation.md).

- Dashboard: `make -C dashboard all test` and `test-sanitize` **PASS** (five
  suites incl. new config/manager and CLI tests; ASan/UBSan with leak detection).
  Saved previews still match CLI output. Host benchmark: prepare 1.0–1.5 us,
  SVG 8.4–11.4 us per frame (host only).
- Upstream `run_tests.sh`: official run fails at check 22 (gcc 13
  `warn_unused_result` in upstream altscreen code). With labelled diagnostic
  adjustments 38/39 pass; `hw301_bridge_tests` unavailable (no IPv6 here).
- Java/stock scripts unavailable (proprietary HMI JAR/JDK 8 absent); native
  maneuver script passes its Linux part, macOS GL link unavailable.
- Fast-forwarded to master on user request. Next eligible step: P2-05 (packaging, previews,
  rollback evidence) when requested, without tests (user instruction).

## Completed — P2-03 (host configuration and manager; merged to master)

User request: "Start P2-03 on a dedicated feature branch". Branch
`feature/p2-03-config-manager`. Added strict versioned config (disabled Compact
default, layout selection independent of enablement, enable/disable/restore-default
actions), a single-provider Dashboard Manager that clears its frame on invalid
config, disabled state, provider failure or unsupported geometry, and two host
CLIs. Details: [config.md](config.md).

Validation: `make -C dashboard all` compiled cleanly with
`-std=c99 -Wall -Wextra -Werror -pedantic`. No tests, demos, CLIs or sanitizers
executed (user's final-stage test deferral). No GEM, upstream runtime, Android or
vehicle code (P2-04 now tests it). Fast-forwarded to master at the user's request ("merge to master");
master therefore includes this compiled-but-untested host code.

## Current publication — PUBLISH-MASTER

User request: "Push everything to main/master on my forked project". The fork's
`master` was fast-forwarded from b127988 to maintenance source commit
`f5bd2e72290a1509d63f831e7dce4dc5282afaa9` and pushed; remote SHA was verified.
This includes P2-01, P2-02, roadmap/provider design, agent audit and MAINT-141.
No conflicts, runtime changes or history rewrites. The final publication checkpoint
is a later documentation commit; use Git history for its SHA. No release or vehicle
execution. See [the audit](audit/LOG.md) for exact authorization and evidence.

## Completed maintenance verification — MAINT-141

Maintenance host tests were explicitly authorized despite the older feature-test
deferral. The original local-only restriction was superseded by the publication
request above. Testing results below are unchanged; no tests rerun for publication.

Dedicated branch: `maintenance/upstream-v1.4.1`, based on `6d5e660`.
The exact requested upstream commit b127988 was already our original baseline.
The complete 20-file release delta remains unchanged; no duplicate integration
or runtime rewrite was needed. New targeted regressions verify release/custom
preservation, renderer policy and v1.4→v1.4.1 program/menu preference upgrades.

Host results: upstream suite passed with its explicit missing shipping-JAR skip;
dashboard C/XML/CLI/saved-preview checks and ASan/UBSan passed; new maintenance
test's four methods passed; renderer identity and FIN/RST recovery passed. Native
macOS GL and proprietary Java checks could not complete on this host. Actual
QNX decoder/vehicle behavior remains unverified. Full commands, limits, source
mapping and future A4 B9 test gates are in
[the maintenance report](../maintenance/upstream-v1.4.1.md).

No phase 3–5 work, new product mock provider, source-interface change, release
artifact or vehicle action. P2-02 visual/in-car acceptance is not established by
these successful host regressions. The five-phase roadmap remains unchanged.

## Prior feature-work checkpoint (historical)

Status date: 2026-11-07. Scope: user-authorized P2-02 / PLAN step 2, host only.
Further tests are deferred until the final stage by explicit user instruction.

Current update: DOC-ROADMAP, documentation/design only. Added the complete
[five-phase roadmap](../ROADMAP.md) and [provider design](data-providers.md).
VehicleDataProvider (Phase 2) and future AndroidDataProvider (Phases 4–5) must
feed one shared manager/widget path. Phase 3's Galaxy S23 Ultra app initially
operates independently. No runtime/source code changed, no Android app/transport
or live adapter added, and no tests or automated checks run for this update.
P2-02 final validation remains deferred; manager/configuration remains P2-03.
Roadmap/provider design published in `b4bba59237d062aa06c3f13261a23319bb47d649`,
with remote source SHA confirmed. This is documentation, not implemented adapters.

## Testing policy during P2-02 (historical, ended by P2-04)

During P2-02 the user instructed: "Sa nu executi teste momentan, testele sunt
pentru final, mergem inainte fara teste. Doar sa documentezi fiecare schimbare."
No further tests or automated validation checks are to run until the final stage
is requested. Continue implementation, record every change and preserve this
policy across agent handoffs. Earlier results below remain historical evidence.
Final validation and visual/readability acceptance are not complete.

## Completed

- Created the full upstream-derived fork and documented architecture, evidence,
  requirements, plan, agent instructions and persistent audit/handoff protocol.
- Implemented P2-01 in `dashboard/`: bounded typed snapshots, provider callback,
  deterministic mock source, six widget formatters, reserved frame command types,
  independent host Makefile/library and text demo.
- Added `tests/dashboard_test.c`: units, negative fractional temperature, valid
  zeros, metadata/provenance, age boundary, extreme/future timestamps, numeric
  bounds, invalid enums, unterminated/control text, partial snapshot behavior,
  provider failure clearing and deterministic mock values.
- Documented the API, units, host policy limits and commands in [module.md](module.md).
- Preserved inherited runtime/source, upstream README and installation payloads.
  No vehicle connection or actions performed; all demo values are explicitly MOCK.

- Implemented P2-02 host Compact/Extended frames, strict draw-list validation,
  escaped/clipped SVG export, explicit mock preview CLI and four saved SVGs.
- Prepared C rendering and independent Python XML/CLI test cases for the final
  stage. They were written and first run before the testing-deferral instruction.
- Documented every P2-02 change in [changes.md](changes.md), with design/API and
  preview limitations in [rendering.md](rendering.md).

Live progress: [audit/CURRENT.md](audit/CURRENT.md). History:
[audit/LOG.md](audit/LOG.md). P2-02 was authorized by "Okay, push, commit and let's go with step 2." An agent
switch must preserve the active implementation scope and the later no-test policy.

P2-01 implementation commit: `0b256c07d736a3f47062c9f4afa751dd5b4a1e20`, pushed and
verified on `origin/feature/custom-dashboards`.

P2-02 implementation/artifacts commit: `2ad7270bc6d7e2e78332b437ac95e835403fe6e5`,
pushed and confirmed on the feature branch. Final tests remain deferred; publication
does not imply validation or vehicle compatibility.

## Not implemented

Configuration/activation manager, GL/QNX renderer, performance benchmark, GEM
controls, live data adapters or vehicle backend. SVG previews exist; no successful
screenshots or final visual/readability validation are claimed. Root `make` targets in later PLAN steps remain planned; actual
commands currently use `make -C dashboard`. No vehicle compatibility is claimed.

## Reproduce the implemented increment

```sh
make -C dashboard all
make -C dashboard previews
```

No new packages, firmware files, QNX SDK or HMI jar are needed for these targets.
Compiled outputs stay under ignored `build/dashboard/`. Reviewable SVG artifacts
are intentionally saved in `dashboard/previews/` and committed with the source.

These build/export commands are for reproducing host artifacts. Test commands
remain documented in module.md/rendering.md for the final stage; do not run them now.

## P2-02 evidence before test deferral

| Earlier command/attempt | Actual result before the no-test instruction |
| --- | --- |
| `make -C dashboard all test previews` | Exit 0. Widget, C layout/export and independent XML/CLI checks passed; four SVGs generated. This is not final acceptance. |
| `make -C dashboard test-sanitize`, outside traced sandbox | Exit 0, both C suites, ASan/UBSan/default leak detection unchanged. |
| `SKIP_JAR_CHECK=1 sh scripts/run_tests.sh`, outside traced sandbox | Exit 0, shipping JAR check explicitly skipped; sanitizer checks unchanged. Dummy LD_PRELOAD fixture warnings were expected. |
| ImageMagick SVG rasterization | Failed because its rsvg-convert delegate is unavailable; no screenshot produced. |
| Sandboxed Chromium CLI screenshots | Exit 133. Outside-sandbox temporary screenshot processes did not finish and were stopped. No successful visual inspection. |
| Playwright local-file preview attempt | Failed with ERR_BLOCKED_BY_ADMINISTRATOR; no bypass attempted and no screenshot/measurement success claimed. |

After the user instruction, no further tests or automated validation checks ran.
Documentation/source review and Git publication are the remaining activities.
No package was installed for screenshots. Four saved mock SVGs are available in
[dashboard/previews/](../../dashboard/previews/); final visual review is deferred.
Local logs under `/tmp/mib2q-p2-02-*` and `/tmp/dashboard-*` are not committed;
the audit preserves the observed outcomes.

## P2-01 validation evidence (historical)

| Command / environment | Actual result |
| --- | --- |
| `make -C dashboard all test demo` | PASS (exit 0), strict C99 warnings as errors; library, unit tests and labeled text demo. |
| `make -C dashboard test-sanitize`, traced sandbox | Failed: LeakSanitizer fatal error under ptrace. This invocation is not a pass. |
| Same sanitizer command outside sandbox | PASS (exit 0), AddressSanitizer/UndefinedBehaviorSanitizer and default leak detection unchanged. |
| `sh scripts/run_tests.sh`, traced sandbox | First five named tests passed; stopped at LeakSanitizer restriction in rgd_lane_cache. |
| Same upstream command outside sandbox | Passed through bus transport and 41 protocol constants, then exit 1: shipping `build/carplay_hook.jar` missing. |
| `SKIP_JAR_CHECK=1 sh scripts/run_tests.sh`, outside sandbox | PASS (exit 0), using upstream's documented CI option; shipping JAR check explicitly skipped. Sanitizers remained enabled. Expected dummy LD_PRELOAD fixture warnings occurred; corresponding test passed. |
| `bash scripts/test_route_info.sh` | Exit 2 with no diagnostic. Stock HMI jar/JCL and stock JDK are absent, so Java validation is unavailable; no passing Java result claimed. |

Original investigation recorded a bus transport assertion in a sandboxed retry
with leak detection disabled. It passed on the current outside-sandbox regression
runs. No upstream source was changed to obtain that result; the exact earlier
failure cause was not established. Do not convert this into a universal claim
about sandbox behavior.

Other Java linkage suites, native macOS GL rendering and QNX cross/vehicle builds
were not run for this isolated module. The proprietary assets/toolchains/platform
prerequisites are unavailable here. The P2-02 host SVG exporter is isolated from those unavailable platform paths.
Local `/tmp/mib2q-p2-01-*.log` files contain these run outputs but are not portable
or committed; this document and the audit preserve the results.

## Repository and rollback

Fork: [alpersid-org/audi-mib2q-dashboards](https://github.com/alpersid-org/audi-mib2q-dashboards).
Published branch: `master`; upstream baseline:
`b127988a68c8d690841f4a75d35717ffc01e668e`. Master includes all existing custom
and maintenance work. Prior feature and maintenance branches are retained.
Push project changes to the fork, never upstream.

`make -C dashboard clean` removes module outputs. The host library/demo is not
integrated into installation or startup, so no head-unit rollback is necessary.
Source changes can be reverted by their feature commit without discarding other work.

## Remaining blockers and next step

1. Target identities are supplied in target-profile.md; Phase 1 is installed (upstream 1.4.1; Phase 1 handover).
2. MMI AA works; enhanced cluster/rich-feed compatibility with the original adapter remains unverified. TWO+ evidence is insufficient.
3. No safe dashboard rectangle/ownership policy has been demonstrated.
4. Real data semantics, validity and availability during AA require validation.
5. Shipping-JAR/Java and vehicle checks remain unavailable, not passed.

P2-01 is complete. P2-02 implementation and artifacts are ready for review;
final testing is deferred. Next implementation task, when requested, is P2-03:
disabled-by-default configuration and manager suppression. Do not run tests in
that increment unless the user changes the final-stage-only testing policy.
No vehicle integration or roadmap Phases 3–5 are authorized.
