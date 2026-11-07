# P2-FINAL: final Phase 2 host testing stage (partial; stopped early)

User request: start the final Phase 2 host testing stage (see audit event -23).
Host only; nothing deployed, installed, flashed or run on the vehicle. Step 9
stays blocked pending firmware and safe-surface evidence. Host: gcc 13.3.0,
Python 3.13, **host JDK 21** (stock JDK 8 and stock HMI JAR absent).

The user asked to commit and push before the stage finished (token budget), so
the inherited upstream regressions were **NOT RUN** in this stage (see below).

## Results

| Check | Result |
| --- | --- |
| `make -C dashboard all test` (6 suites incl. new `dashboard_vehicle_test`) | **PASSED** |
| `make -C dashboard test-sanitize` (ASan/UBSan, leak detection on, 4 C suites) | **PASSED** |
| `make -C dashboard test-java` (DashboardConfigTest, AaRgStateDashboardTest, DashboardModuleTest, Java→C fixture) | **PASSED**, 5/5 repeated runs |
| `bash scripts/test_app_lifecycle.sh` (R4 repaired harness, 6 scenarios) | **PASSED** on JDK 21 |
| `make -C dashboard benchmark` | Completed: prepare 0.99–1.31 µs, SVG 8.1–11.4 µs/frame (host only); Extended 42/48 commands |
| Mutation checks: reintroduced R1, R2, R3, no writer timeout | Each **caught** by its test; sources restored |
| Baseline before repair | dashboard_test.c did not compile; render/manager tests FAILED on old Extended content; R4 isolated compile FAILED (reproduced) |
| `SKIP_JAR_CHECK=1 sh scripts/run_tests.sh`, `tests/upstream_v141_test.py`, `test_maneuver_native.sh` | **NOT RUN** (stopped early). Last results: P2-04 (validation.md), historical |
| Stock-JAR Java scripts (incl. full `test_java_transports.sh`, VehicleDataModule DSI gating) | **UNAVAILABLE**: stock HMI JAR/JDK 8 absent |

## R1–R5 recheck

| Finding | State |
| --- | --- |
| R1 write-time order | VERIFIED BY HOST TESTS (DashboardModuleTest; mutant caught) |
| R2 late publication | VERIFIED BY HOST TESTS (stop during collection and during a blocked FIFO write; restart; mutant caught) |
| R3 reader clock order | VERIFIED BY HOST TESTS (dashboard_vehicle_test interleaving; mutant caught) |
| R4 lifecycle harness | REPAIRED and PASSED (isolated, JDK 21); full stock-JAR script UNAVAILABLE |
| R5 docs | Implemented earlier; this report and status updated |
| Session liveness | VERIFIED BY HOST TESTS (mutant caught) |
| AA Kombi-only binding | UNAVAILABLE (needs stock DSI classes) |

## Changes in this stage

- Tests updated: dashboard_test.c, dashboard_render_test.c, dashboard_svg_test.py,
  dashboard_manager_test.c, CarPlayAppLifecycleTest.java, test_java_transports.sh.
- Tests added: dashboard_vehicle_test.c, DashboardConfigTest.java,
  DashboardModuleTest.java, AaRgStateDashboardTest.java, stubs in
  tests/stubs/dashboard-feed and tests/stubs/app-lifecycle (FrameworkRef.java,
  dashboard/DashboardModule.java), scripts/test_app_lifecycle.sh,
  scripts/test_dashboard_java.sh, Makefile targets (vehicle test, test-java).
- Previews: Extended pair regenerated; Compact unchanged (byte-identical).
- Production: package-private `DashboardConfig.path` / `DashboardModule.recordPath`
  for tests (default to the real paths).

## Next

Run the inherited regressions (`run_tests.sh`, `upstream_v141_test.py`,
`test_maneuver_native.sh`) and record results; then decide merge/step 9 gates.
