# Project status

## Current feature work — P2-03 (host configuration and manager)

User request: "Start P2-03 on a dedicated feature branch". Branch
`feature/p2-03-config-manager`. Added strict versioned config (disabled Compact
default, layout selection independent of enablement, enable/disable/restore-default
actions), a single-provider Dashboard Manager that clears its frame on invalid
config, disabled state, provider failure or unsupported geometry, and two host
CLIs. Details: [config.md](config.md).

Validation: `make -C dashboard all` compiled cleanly with
`-std=c99 -Wall -Wextra -Werror -pedantic`. No tests, demos, CLIs or sanitizers
executed (user's final-stage test deferral). No GEM, upstream runtime, Android or
vehicle code. Fast-forwarded to master at the user's request ("merge to master");
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

## Active testing policy

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

1. Exact firmware/hardware and Phase 1 success remain unconfirmed.
2. Original AAWireless compatibility is unknown; TWO+ evidence is insufficient.
3. No safe dashboard rectangle/ownership policy has been demonstrated.
4. Real data semantics, validity and availability during AA require validation.
5. Shipping-JAR/Java and vehicle checks remain unavailable, not passed.

P2-01 is complete. P2-02 implementation and artifacts are ready for review;
final testing is deferred. Next implementation task, when requested, is P2-03:
disabled-by-default configuration and manager suppression. Do not run tests in
that increment unless the user changes the final-stage-only testing policy.
No vehicle integration or roadmap Phases 3–5 are authorized.
