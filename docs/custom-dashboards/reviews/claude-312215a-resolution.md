# Resolution of the 312215a review

**Final host validation (supersedes the historical states below):** see
[validation-final.md](../validation-final.md). R1/R3 and the repaired R4 harness
pass host checks. Codex reproduced a residual R2 defect with a prior successful
record and a blocked second write, then fixed it by ordering final rename with
stop and always removing the prior record. Its strengthened regression passes.
R5 current guides and four previews are reconciled. Stock Java/DSI/QNX and VC
safety remain unverified; Phase 2 is not complete. The earlier resolution table
records the original implementation at 9187a88, not current validation status.

Resolution record for [the review of 312215a](claude-312215a.md), which stays
pinned and unchanged. Implemented by Claude Code on 2026-11-07, branch
`fix/review-claude-312215a` (from `review/claude-phase2`, source identical to
312215a). User request: "check this audit ... implement all fixes".

**Validation:** manual source review only. No builds, tests, test code, demos,
benchmarks, preview generation or automated validation, under the standing user
instruction ("do not run any test, there will be a ask at the end to implement
and run the tests") and the handoff rule not to build as a substitute for
testing. Nothing below is verified by execution; nothing ran on the car.

| Finding | State | Change | Manual evidence | Deferred checks |
| --- | --- | --- | --- | --- |
| R1 P1 producer timestamps | IMPLEMENTED / UNTESTED | `DashboardModule.writeLoop` reads owner, navigation and vehicle snapshots first, then takes `written`. Reader validation unchanged. | `dashboardVehicle()` stamps with its own read time and `AaRgState` times come from earlier trip samples; both precede `written` in program order on the same clock. | Clock tick between reads and write time; Java-encoded record parsed by C. |
| R2 P2 late publication after stop | IMPLEMENTED / UNTESTED | Writer generations; one in-flight publication at a time, allowed only for the current generation; I/O outside the lock; `stop()` bumps the generation and removes the record unless a publication is in flight, which then removes its own record after rename; per-generation temp files. | `stop()` waits on no I/O (only the generation bump and small deletes under the lock). After `stop()`, either the record is already gone, or the in-flight writer deletes it in its `finally` block, and no other generation could publish meanwhile. | Stop during collection/write/rename; immediate restart; write failure recovery. |
| R3 P2 reader clock order | IMPLEMENTED / UNTESTED | `db_vehicle_read` samples the wall clock after the record is fully read. | The future-time check now compares against a clock taken after any publication that the read observed. Ages err older. | Publication between clock sample and open; age/future boundaries. |
| Session invalidation (report "known gaps") | IMPLEMENTED / UNTESTED | DBV1 bytes 60–63 carry a non-zero writer session (process nonce mixed with generation); sequence starts at 1. Reader rejects zero session/sequence; with a context it tracks session, rejects backwards sequences and fails when the newest record has not advanced for 2 s. | Covers disconnect (record removed at stop, R2), HMI death (2 s), same-owner reconnect and re-enable (new session), reader restart (stateless parse; tracking restarts). | Session change, backwards sequence, 2 s timeout, reader restart. |
| AA subscriptions (report "known gaps") | IMPLEMENTED / UNTESTED | During Android Auto, `VehicleDataModule` binds only `DSICarKombi` and starts no sender thread when the dashboard is enabled. CarPlay unchanged. | General/dynamic bindings and the sender loop are skipped on the dashboard-only path; `stop()` releases unbound bindings safely. | Gating per session owner; missing Kombi service retry. |
| R4 P2 Java lifecycle harness | OPEN (deferred to the final test request) | None: it is test-harness code (`tests/stubs`, `test_java_transports.sh`, `CarPlayAppLifecycleTest`). The review queue also places it at the final test stage. | Still applicable: `CarPlayApp` constructs `DashboardModule`; the isolated harness lacks it, its `DashboardVehicle`/`dashboardNav` dependencies and the new owner policy. | Isolated module fixture, owner/gating assertions, real `DashboardModule` lifecycle. |
| R5 P2 documentation drift | IMPLEMENTED (documentation) | PROJECT, data-providers, architecture, package, module, rendering, ROADMAP and vehicle-provider now state the modified shipping Java, the implemented manager/provider, two remaining previews, the active no-test policy and session-bound disabling; older claims are pinned to their revision. | Manual comparison with the source after these fixes. | — |

## Changed paths

- `java_patch/com/luka/carplay/dashboard/DashboardModule.java` (R1, R2, session)
- `java_patch/com/luka/carplay/core/VehicleDataModule.java` (AA Kombi-only)
- `dashboard/vehicle_provider.c`, `dashboard/dashboard.h` (R3, session/liveness)
- Documentation listed under R5, plus this record, the review index, audit and
  change ledger.

## Remaining before a vehicle renderer

- Final test request: repair R4 and the stale suites (P2-07), and add the
  feed, Java/C and lifecycle cases listed in
  [vehicle-provider.md](../vehicle-provider.md).
- Full Java 1.4 / stock linkage is still uncompiled here (stock JAR absent).
- Media still has no verified Android Auto source; CarPlay navigation is not
  wired; layouts still allocate a media card (step 9 decision).
- Step 9 still needs firmware/ABI evidence and a proven non-critical surface,
  not only a surface name. This record authorizes no deployment.

## P2-FINAL update

R1, R2, R3 and session liveness: VERIFIED BY HOST TESTS (mutation checks). R4: repaired, isolated harness PASSED on JDK 21; full stock-JAR script UNAVAILABLE. AA Kombi-only binding: UNAVAILABLE (stock DSI). See [validation-final.md](../validation-final.md).
