# P2-04: host validation and regression report

PLAN step 4, the final testing stage. User request: "Start P2-04, but before make
sure you update the documentation, as instructed". The documentation/scope update
was committed (88d68ea) before any test ran. Run date 2026-11-07 on branch
`feature/p2-04-validation`, base master 12cb0b5.

Host: Linux container, 4 CPUs, gcc 13.3.0 (Ubuntu 24.04), Python 3.13.18, no IPv6,
no proprietary stock HMI files. **Host results do not show vehicle compatibility,
safe Virtual Cockpit composition or head-unit performance.**

## Dashboard suite (new and existing)

| Command | Result |
| --- | --- |
| `make -C dashboard all test` | **PASS**, exit 0. Five suites below. |
| `make -C dashboard test-sanitize` | **PASS**, exit 0. ASan+UBSan, `-fno-sanitize-recover=all`, `ASAN_OPTIONS` unset (leak detection on). |
| `make -C dashboard benchmark` | **Completed**, exit 0. Numbers below. |

| Suite | Covers |
| --- | --- |
| `dashboard_test` (P2-01) | Units, zero values, validity/provenance, freshness boundaries, numeric/text bounds, provider clearing, mock determinism. |
| `dashboard_render_test` (P2-02) | Layout content and bounds, crops and clearing, placeholders/elision, malformed frames, XML escaping, I/O errors. |
| `dashboard_svg_test.py` (P2-02) | Independent XML parse of 4 profiles, clips, MOCK badges, deterministic bytes, CLI errors; **saved previews are byte-identical to CLI output**. |
| `dashboard_manager_test` (**new**, P2-03) | Config parser: 8 accepted forms, 31 rejected forms, NUL byte, 64-byte line cap, 512-byte input cap, NULL inputs; validity; actions (layout keeps enablement, disable keeps layout, invalid → default); write/parse round trip and `/dev/full`; loader missing/valid/invalid/oversize/directory/NULL. Manager: default disabled with **zero provider reads**; layout selection alone never enables; layout switch clears then rebuilds; provider results −1/0/2 clear and recover; guidance crop 210x153 and viewport outside canvas clear; disable stops reads; invalid config sticky `invalid-config` with no reads until restore; restore-default from READY; corrupted state caught; NULL provider/manager safety; status names. |
| `dashboard_cli_test.py` (**new**, P2-03) | `dashboard_config`: default/missing/invalid `show`, all six actions, recovery from invalid input, usage exit 2, output round trip. `dashboard_manager_preview`: missing/invalid/disabled config → exit 3 with **empty stdout** and reason; enabled Compact/Extended wide/tall → valid deterministic SVG with 4/6 MOCK badges and host banner; usage exit 2. |

Mutation check (to show the new tests can fail): making `db_config_select_layout`
also enable, and removing frame clearing on provider failure, were each caught
by `dashboard_manager_test`; sources were restored with `git checkout`. Removing
the clear on the disabled path in `db_manager_update` was not detected, because
every action already clears the frame first. That path is redundant safety, not
an untested defect.

### Host benchmark

Fixed 20000 manager updates (provider read + layout build) and 2000 SVG writes
to `/dev/null` per profile, mock provider, `-O2`:

| Profile | Prepare | SVG export | Commands |
| --- | --- | --- | --- |
| compact-wide 800x252 | 1.02 us/frame | 8.38 us/frame | 30 |
| compact-tall 640x360 | 1.02 us/frame | 8.50 us/frame | 30 |
| extended-wide 800x252 | 1.48 us/frame | 11.43 us/frame | 42 |
| extended-tall 640x360 | 1.44 us/frame | 11.38 us/frame | 42 |

`sizeof(db_manager)` = 6448 bytes; no heap use. Extended uses 42 of the 48
command slots (`DB_COMMAND_CAP`): adding widgets needs a capacity review. These
are timings on this host only. They must not be used to infer MMI CPU load or
frame rate.

## Inherited upstream scripts

| Command | Result |
| --- | --- |
| `SKIP_JAR_CHECK=1 sh scripts/run_tests.sh` | **FAIL**, exit 1. Checks 1–21 PASS; check 22 `alt111_settings_test` does not compile: gcc 13 here enforces `warn_unused_result` on `write()` despite `(void)` casts in upstream `altscreen_hook/libaltscreen111_gen2.c`, under `-Werror`. MAINT-141 passed all 39 on a different host; this is a toolchain difference in upstream code, which was not modified. |
| Diagnostic: same, with a scratch `cc` wrapper adding `-Wno-error=unused-result` | exit 134. `alt111_settings_test` and `alt111_control_test` PASS; `hw301_bridge_tests` aborts: the test needs an IPv6 socket and this container has no IPv6 (`EAFNOSUPPORT`, errno 97, reproduced with Python). **Unavailable here**, not a code verdict. |
| Diagnostic: wrapper + scratch copy omitting only `hw301_bridge_tests` | exit 0. Remaining 16 checks PASS (AA navxlate/trip/uiconfig/gate/profile, maps, tel, vehicle, nav boost, AA tee incl. SIGPIPE, yield, hw_yield files, tee port, startup, RGI persistence). |
| `bash scripts/test_maneuver_native.sh` | exit 1. Independent lane receiver PASS; then the macOS-only GLFW/OpenGL parity link (`-framework OpenGL`) fails on Linux. Unavailable on this host. |
| `bash scripts/test_route_info.sh`, `test_java_transports.sh`, `test_mmi_settings.sh`, `test_pdc.sh`, `test_lane_guidance_e2e.sh`, `audit_maneuvers.sh`, `audit_java_stock.sh` | each exit 2, no output. `stock/` holds only its README; `scripts/stock_env.sh` exits under `pipefail` on the empty `stock/libs/jcl/*` glob before its own "missing jar" message. Proprietary stock HMI JAR, J9 class library and host JDK 8 are absent. **Unavailable**; nothing was downloaded. |

In total, with the two labelled diagnostic adjustments, 38 of the 39
`run_tests.sh` checks pass on this host and one is unavailable (no IPv6). The
unadjusted official run fails at check 22 for the toolchain reason above.
No sanitizer was disabled, no failure was relabelled as a skip, and no upstream
or production file was changed to make a test pass.

## Acceptance (PLAN step 4)

- New host suite passes: **met** (`test` and `test-sanitize`).
- Coverage: rendering semantics, widget units/bounds, mock determinism,
  configuration, layout compatibility, clearing and fallback, independent SVG
  parsing: **met** by the five suites.
- Bounded host benchmark: **met** (`make -C dashboard benchmark`).
- Upstream passes/failures/unavailable checks recorded with causes: **met**
  (table above).
- Not covered (out of host scope): visual readability review on a real cluster,
  multi-provider resolution (not implemented), live providers, QNX build,
  stock Java integration, any vehicle behaviour.

Target names adjustment: PLAN step 4 lists root-level `make test`,
`make test-sanitize` and `make benchmark`. They are implemented as
`make -C dashboard <target>` instead, to avoid adding a root Makefile that could
be confused with upstream's own build scripts.

## Reproduce

```sh
make -C dashboard all test
make -C dashboard test-sanitize
make -C dashboard benchmark
SKIP_JAR_CHECK=1 sh scripts/run_tests.sh
```

Rollback: `make -C dashboard clean` removes all outputs, including test fixtures.
