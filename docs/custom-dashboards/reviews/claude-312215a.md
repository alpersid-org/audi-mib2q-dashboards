# Review of Claude's pushed Phase 2 work

AI continuation: read [the pickup instructions and corrective queue](README.md)
and [the live checkpoint](../audit/CURRENT.md). Findings R1–R5 remain open;
this report is a revision-pinned review, not evidence of implemented fixes.

Reviewed by Codex on 2026-11-07. Requested scope: "Review everything pushed by
Claude". Review range: `574ae2674ec08a05c8ea233bf516ea2e5f35b450` through
`312215a` (15 commits, 45 changed paths). All fetched origin feature/documentation
branches are included in origin/master at this revision. Local review branch:
`review/claude-phase2`. It was prepared locally; no code fixes, commits, merges
or pushes were performed during review. The user subsequently authorized
committing/pushing these documents; see the live checkpoint for publication state.

**Assessment: changes needed before connecting a vehicle renderer.** The host
foundation remains modular, but the new P2-08 feed has timestamp and shutdown
races. Phase 2 is not a working Virtual Cockpit feature yet.

This was a manual source, diff and documentation review, including upstream
lifecycle/build consumers. No builds, tests, test code, demos, benchmarks,
preview generation or automated validation were run, following the latest
recorded user instruction. Findings below follow from source execution order;
their frequency and on-car effects have not been measured.

## Findings

### R1 — P1: valid vehicle samples can be newer than their record's write time

Location: [DashboardModule.java:88](../../../java_patch/com/luka/carplay/dashboard/DashboardModule.java#L88).
Related: [VehicleDataModule.java:181](../../../java_patch/com/luka/carplay/core/VehicleDataModule.java#L181),
[vehicle_provider.c:76](../../../dashboard/vehicle_provider.c#L76).
Introduced by `0d4448c`.

Java evaluates call arguments left to right. `writeLoop` captures the `now`
argument before calling `VehicleDataModule.dashboardVehicle()`. That accessor
captures another `System.currentTimeMillis()` and uses it for valid temperature
and range sample times. If the millisecond advances, `tempMs` or `rangeMs` is
larger than the record's `written` field. The C reader rejects the entire record
when any sample time exceeds `written`.

For example, a write time of 100000 and a vehicle-read time of 100001 produces a
checksummed record the reader must reject. This also drops otherwise valid
navigation. Through the manager the result is OFF_PROVIDER and an empty frame.

Collect the source snapshots first, then timestamp the completed record. Keep
future/corruption checks; do not hide the bug by relaxing validation. At the
final authorized testing stage, cover a clock tick between argument evaluations
and a Java-produced record parsed by C.

### R2 — P2: stop can be followed by a late publication from the old writer

Location: [DashboardModule.java:70](../../../java_patch/com/luka/carplay/dashboard/DashboardModule.java#L70),
with the in-flight path at lines 83–90 and publication at lines 153–163.
Introduced by `0d4448c`.

`stop()` interrupts the thread and deletes both files, but does not wait for the
writer or serialize deletion with publication. If the writer already passed
`if (!running) return`, stop can remove the files while that iteration is
collecting/encoding. Ordinary FileOutputStream work and `renameTo` do not provide
a cancellation check here, so the iteration can recreate the final record after
stop returns. Rapid restart also lets an old iteration touch the new writer's
shared `.tmp` path.

The C reader accepts a fresh remaining record for up to 30 seconds. It only
checks that the owner byte is at most 2; it has no current-session check. Thus
file deletion is not currently a reliable disconnect/suppression boundary.

Give each writer an immutable lifecycle generation, prevent old generations
from publishing, and coordinate final removal with the last permitted write.
Use bounded lifecycle coordination so a stuck file operation cannot indefinitely
hold the HMI worker. Add stop-during-collection/publication and immediate-restart
coverage when tests are authorized. No renderer currently consumes this feed;
this is a defect in the new feed's contract, not an observed cluster event.

### R3 — P2: the reader's clock is captured before the record it validates

Location: [vehicle_provider.c:110](../../../dashboard/vehicle_provider.c#L110),
read at lines 118–124 and future-time rejection at line 70.
Introduced by `0d4448c`.

`db_vehicle_read` obtains `wall_now` before opening/reading the file. If Java
publishes a new record between those operations, the reader can read a record
written after that earlier clock value and reject it as future data. This is
independent of R1: it occurs even with a correctly ordered producer.

For example, the reader samples 100000, Java publishes at 100001, then fopen
opens that new inode. A valid record is rejected and the manager clears its
frame. Sample the validation wall clock after acquiring the complete record;
keep the caller-clock age mapping conservative and documented. At the final
testing stage, exercise a publication between the clock sample and file open.

### R4 — P2: the isolated Java lifecycle harness cannot resolve the new module

Location: [CarPlayApp.java:50](../../../java_patch/com/luka/carplay/core/CarPlayApp.java#L50).
Related: [test_java_transports.sh:79](../../../scripts/test_java_transports.sh#L79),
[LifecycleFixtures.java](../../../tests/stubs/app-lifecycle/com/luka/carplay/core/LifecycleFixtures.java),
[CarPlayAppLifecycleTest.java:125](../../../tests/CarPlayAppLifecycleTest.java#L125).
Introduced by `0d4448c`.

CarPlayApp now unconditionally constructs DashboardModule. The existing isolated
lifecycle javac invocation supplies selected source files and stub modules, but
no dashboard module, source path or shipping-JAR classpath. Under its documented
setup, it cannot resolve the new fully qualified class. Supplying the real module
alone is insufficient: it uses DashboardVehicle and other APIs absent from the
isolated stubs. The owners assertion also still requires VehicleDataModule not
to start in Android Auto, contrary to the new OWNER_ANY policy.

This is separate from missing proprietary assets: restoring those assets does
not repair this isolated invocation. The final test stage needs an isolated
module fixture and owner/gating assertions matching the intended session rules,
plus real DashboardModule lifecycle coverage. No tests were run or modified in
this review; this is an additional source-visible verification blocker beyond
the P2-07 dashboard-test debt already documented.

### R5 — P2: current architecture/package instructions contradict P2-08 and policy

Locations: [PROJECT.md:17](../../../PROJECT.md#L17),
[PROJECT.md:92](../../../PROJECT.md#L92),
[data-providers.md:21](../data-providers.md#L21),
[architecture.md](../architecture.md), [package.md](../package.md), [ROADMAP.md](../../ROADMAP.md).
Introduced by incomplete documentation reconciliation across the new milestones.

Current entry points still say inherited Java/runtime is unchanged, live adapters
are unimplemented, manager work is pending, and tests may now run. P2-08 modifies
four inherited Java files and adds two HMI classes; `scripts/build_java.sh` lines
51–65 compiles every Java source into the shipping JAR. The C renderer is still
host-only, but a future normal Java build already includes the new HMI module.
That distinction matters for review and rollback.

The package page also claims four saved previews and no code changes since
P2-04, while P2-07 removed both Extended SVGs and changed widgets. The vehicle
provider guide says configuration is only read at session start, yet its last
line says deleting the configuration disables the feature without mentioning
that a currently running session continues. The new no-test policy is present
in AGENTS, but other current guides still instruct running tests.

Reconcile current-state documents with 312215a, retaining older commands/results
as explicitly pinned historical evidence. Describe changed Java shipping content,
session-bound disable behavior and the remaining two previews. Preserve the
append-only audit rather than rewriting old events. This is material to reliable
agent handoff and the user's requirement to document each change.

## Milestones and architecture assessment

| Work | Commits | Review assessment |
| --- | --- | --- |
| Resume/publication audit | `2d4a255` | Continues the existing audit. |
| P2-03 config/manager | `72d3df5`, `12cb0b5` | Bounded parser, disabled default, generic injected provider, clearing on failure and layout changes. No additional functional defect identified by manual review. |
| P2-04 tests/benchmark/report | `88d68ea`, `85c523e`, `8e49267`, `f7e9fb2` | Reports actual historical passes and failed/unavailable upstream checks separately. Later no-test policy is explicit. Results do not validate subsequent P2-07/P2-08 code. |
| P2-05 package | `0102e71`, `492b84a` | Useful pinned host package; current-state instructions have drifted (R5). |
| Phase 2 scope | `1ff58d9`, `f89a58c` | Correctly distinguishes the host foundation from a complete in-car feature; five-phase boundaries preserved. |
| P2-07 widgets | `1e2cc2b`, `a256758` | Removes unsupported trip/consumption, uses existing final-destination semantics and common widgets. Existing tests/previews deliberately left unfinished under the test deferral. |
| P2-08 vehicle feed | `0d4448c`, `312215a` | Reuses existing DSI conversions/AA trip resolution, shared C callback and explicit UNVERIFIED provenance; timing, lifecycle and verification problems above need resolution. |

No native renderer, hook, installer, deploy or GEM implementation files changed
in this review range. Upstream v1.4.1 remains inherited; this review found no
removal of its native AA tee/SIGPIPE or decoder coordination implementations.
That is source inspection, not a new regression pass. No Android app or new
Android-to-MMI communication was introduced.

## Known gaps and next gates

- Tests are deferred by user. `dashboard_test.c` still references removed types/
  fields and cannot compile against the current header. Rendering/SVG/manager
  assertions still describe the removed widgets; the maintenance audit's frozen
  custom-tree equality check also needs a revision-aware scope. These are known
  or directly visible test debts, not newly executed failures. P2-08 has no
  Java/C cross-language or lifecycle tests yet.
- Session identity is still incomplete: DBV1 has an owner type and a sequence,
  but the reader does not track either against an active session/generation.
  Define invalidation across disconnect, HMI/reader restart and same-owner
  reconnect before wiring a consumer; timestamps alone are not session identity.
- Media has no supplied real AA source; CarPlay navigation is not wired. Both
  layouts still allocate a media card. The req. 8 omission policy remains a
  step-9 gap, already acknowledged in vehicle-provider.md.
- VehicleDataModule now binds general/dynamic services as well as Kombi in an
  enabled AA session, despite dashboards consuming only temperature/range.
  This reuses existing attributes and issues no vehicle-control command, but
  its extra subscriptions/sender work and missing-service behavior still need
  justification and regression coverage before enabling on hardware.
- Full shipping Java 1.4/stock linkage and QNX decoder/display behavior are
  unverified. Compiling DashboardConfig at Java 8 level does not prove the HMI
  module links/runs against the target stock firmware.
- No safe VC rectangle, firmware/hardware compatibility or successful Phase 1
  install has been established. Keep the QNX guard and disabled default. A user
  surface preference does not establish safe geometry or OEM navigation ownership.

Recommended order: resolve R1–R3 and session invalidation on the host; reconcile
R5; at the explicitly requested final test stage repair the existing suites and
R4, then validate Java/C encoding, stop/restart and clock-boundary behavior.
Only after independent firmware/surface evidence should step 9 advance toward
vehicle rendering. This review does not authorize deployment or installation.
