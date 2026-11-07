# Project status

Status date: 2026-11-07. Scope: user-authorized P2-01 / PLAN step 1, host only.

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

Live progress: [audit/CURRENT.md](audit/CURRENT.md). History:
[audit/LOG.md](audit/LOG.md). Scope was authorized by the user's instruction
"Okay, go ahead with the first part" following the ordered plan; an agent switch
must not reimpose the previous documentation-only scope on P2-01.

## Not implemented

Compact/Extended layouts, SVG/GL renderer, configuration/activation manager,
graphics previews, performance benchmark, GEM controls, live data adapters or
vehicle backend. Root `make` targets in later PLAN steps remain planned; actual
commands currently use `make -C dashboard`. No vehicle compatibility is claimed.

## Reproduce the implemented increment

```sh
make -C dashboard all test demo
make -C dashboard test-sanitize
```

No new packages, firmware files, QNX SDK or HMI jar are needed for these targets.
Generated files stay under ignored `build/dashboard/`.

## P2-01 validation evidence

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
prerequisites are unavailable here. No host graphics renderer exists yet to test.
Local `/tmp/mib2q-p2-01-*.log` files contain these run outputs but are not portable
or committed; this document and the audit preserve the results.

## Repository and rollback

Fork: [alpersid-org/audi-mib2q-dashboards](https://github.com/alpersid-org/audi-mib2q-dashboards).
Branch: `feature/custom-dashboards`; upstream baseline:
`b127988a68c8d690841f4a75d35717ffc01e668e`. Master remains inherited upstream.
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

P2-01 acceptance is complete. Next eligible task, when requested, is P2-02:
Compact/Extended frame builders and a host SVG adapter using the module contracts.
No vehicle integration or roadmap Phases 3–5 are authorized.
