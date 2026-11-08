# Phase 2 remaining implementation plan

Current plan: 2026-11-08. Reviewed baseline:
`896cd9bd90a3bb70ae79aeb3f6c426d5cc81f4ce` on fork master.

**Outcome:** working Compact and Extended dashboards inside this A4's Virtual
Cockpit, using accepted real MMI data, with safe controls, persistence, OEM
restoration, a validated target package and actual vehicle acceptance. Phase 2
remains NO-GO until those conditions have evidence.

This is the active completion plan. The original numbered steps in [PLAN.md](PLAN.md)
remain historical implementation context. [completion-audit.md](completion-audit.md)
records fixes already shipped and their host tests. Work packages below are
subtasks of roadmap Phase 2, not new roadmap phases.

## Target evidence update — user supplied

The [target profile](target-profile.md) now records P5092/MU1329, the exact media
driver, VC Classic/Extended Map, Galaxy S23 Ultra and original AAWireless
cs307-e2-prod / 3.5.0. Android Auto is working on MMI. **Phase 1 is NOT INSTALLED.**
C02's identity inventory is partially complete; do not keep requesting the already
supplied identifiers or mark the full work package complete. Exact stock ABI,
original-solution baseline, real signal behavior and safe geometry remain open.

The first hardware gate is now explicit: review/build/package/recovery preparation
for the original runtime, then separately authorized Phase 1 installation and
validation, before target dashboard validation. The fork's current Java runtime
contains dashboard edits and must not be mistaken for the original Phase 1 payload.
Independent C01/C05 host work and source-level C03 analysis can proceed now.

## Decisions and boundaries

These are proposed implementation choices made for the remaining work, not claims
that these features already exist or additional historical requirements:

- Initial dashboard support targets the documented Android Auto session on the
  A4 B9/2017 and original AAWireless. Preserve the inherited CarPlay/OEM baseline.
  CarPlay-specific dashboard navigation and phone-free dashboard operation are
  not closure requirements unless the support contract is explicitly expanded.
- Begin with signals actually accepted on the target. Temperature, driving range
  and AA destination/distance/duration are candidates, not guaranteed capabilities.
  Unsupported media/trip/consumption remain omitted. A smaller verified signal set
  is acceptable; a mock-only or entirely empty dashboard is not a deliverable.
- Keep the shared `db_provider -> db_snapshot -> db_manager -> db_frame` path.
  Add a bounded consumer/backend and lifecycle controller, not another widget or
  data model. Prefer existing Screen/EGL/GLES and lifecycle primitives where the
  target evidence supports them. Choose process/surface hosting at the surface
  gate; neither a new displayable nor the existing 98/99 is assumed available.
- Keep default OFF. Layout selection never enables the feature. Disable/defaults,
  invalid configuration, unsupported target/geometry and loss of safe ownership
  suppress drawing and restore OEM composition.
- Use the existing 500 ms publication period and 2 s stateful writer timeout as
  initial engineering settings, not hardware timing guarantees. Define measured
  target resource/timing limits before accepting the integrated feature.
- No Android companion, new phone transport, companion telemetry, DPF feature,
  vehicle-control logic, guessed ABI offsets or dashboard key interception.

Planning, host code and target build preparation do not authorize installation,
head-unit commands, flashing or vehicle tests. The existing boundary remains in
force. Vehicle evidence/acceptance below is performed by the user or a qualified
operator through a separately authorized procedure. No new permission is needed
to execute independent authorized host work.

## Current evidence

| Area | Evidence at baseline | Remaining work |
| --- | --- | --- |
| Core layouts/manager/provider | `dashboard/`; four SVG previews; source corrections and host tests in completion-audit.md | Production consumer, target drawing and ownership lifecycle |
| Actual output | `dashboard/dashboard.h` rejects QNX; SVG is the only dashboard backend | Native integration after target/surface gate |
| Publisher | `DashboardModule.java` publishes DBV1 every 500 ms, unique staging and generation-safe restart | Real source acceptance, session/validity policy and configuration reconciliation |
| Vehicle readings | `VehicleDataModule.readDashboard()` timestamps cached values at read time; `Binding.isBound()` checks a handle | Validate change-driven semantics and service-loss invalidation; do not assume live health |
| Controls | C actions exist; Java config exposes startup enablement only | Typed shared settings contract and actual MMI controls |
| Build/package | `scripts/build_all.sh` stages inherited renderers; no dashboard consumer | Exact-target assets/linkage, dashboard payload and rollback registration |
| CI | Workflow contains new suites; read-only GitHub Actions run listing returned total_count=0 on 2026-11-08 | Establish workflow execution and inspect results; no evidence yet that CI passed or is disabled |
| Installation | Conflict backups and journal/recovery pass ten host fault-test methods | Target filesystem/storage/permissions and actual rollback validation |
| v1.4.1 | All 20 release-delta blobs preserved at baseline; host behavioral regressions pass | Actual startup/decoder/coexistence validation after integration |

## Work order

| ID | Deliverable | Dependencies | Can start now? | Complexity |
| --- | --- | --- | --- | --- |
| P2-C01 | Reproducible baseline and observed CI results | Published corrections | Yes: source and service diagnosis | Small |
| P2-C02 | Exact target profile, build prerequisites and baseline evidence | User identity supplied; Phase 1 NOT INSTALLED | Identity inventory partly complete; stock ABI/assets and original baseline required to finish | Medium |
| P2-C03 | Verified candidate display geometry and ownership design | C02 target identity | Source analysis now; final surface decision blocked on evidence | Large / investigation |
| P2-C04 | Accepted signal/capability and freshness policy | C02 real DSI/receiver evidence | Host policy/tests now; source promotion gated | Medium |
| P2-C05 | Host-testable production consumer/controller contract | Existing core; C04 contract | Yes, using explicit host profiles and fake platform | Medium |
| P2-C06 | VC drawing backend and native display lifecycle | C02, C03, C04, C05 | Target integration blocked on gates | Large |
| P2-C07 | MMI controls and persistent runtime settings | C05 controller and safely gated C06 consumer | Schema/controller tests now; GEM integration after consumer | Medium |
| P2-C08 | Shipping build, package and recovery validation | C02, C04, C06, C07; green host evidence | Baseline build preparation now; final package later | Medium |
| P2-C09 | Actual vehicle acceptance and closure dossier | C01-C08; authorized vehicle procedure | No: prerequisites incomplete | Large / hardware validation |

C01 and C02 evidence collection can proceed alongside C04/C05 host work. C03 is the
critical feasibility gate for native output. A failed gate changes the plan to
BLOCKED with an explicit reason; it never becomes an invented display mapping.

## P2-C01 — reproducible baseline and CI

**Purpose:** distinguish prior local passes, remote CI evidence and target builds.

1. Preserve source and test baseline IDs, current test outcomes and unavailable
   prerequisites. Do not repeat already-passing suites just to produce activity.
2. Determine whether Actions is enabled/discovering this workflow and whether
   repository policy or execution permissions prevent runs. Zero runs is not a
   diagnosed cause. Use available API/UI evidence; do not silently change settings.
3. Execute the existing workflow at the selected implementation revision, inspect
   every job/result/log and repair actual failures. Confirm ksh/mksh fixture paths
   execute, dashboard C/sanitizer/Java/feed/lifecycle/recovery suites run, and pinned
   upstream release tags are available to maintenance checks.
4. Before legitimate edits to inherited release files such as the GEM/release
   builders, update `tests/upstream_v141_test.py` deliberately: pin historical
   blob identity to the verified integration revision, while retaining current
   SIGPIPE/yield/policy/upgrade behavior tests. Do not freeze current files forever
   or weaken behavior assertions merely to accommodate failures.

**Files:** `.github/workflows/tests.yml`, existing test/build scripts and
`tests/upstream_v141_test.py` when later changes require its historical scope.

**Acceptance:** a successful observed run for the relevant code revision, linked
run/job evidence and explicit stock/hardware exclusions. Local-only success is
recorded separately. If external execution remains unavailable, report that gate
as unresolved instead of inventing a successful run.

## P2-C02 — target profile, assets and baseline

**Purpose:** prevent implementing and packaging against another car's ABI.

Identity evidence is recorded in [target-profile.md](target-profile.md). Treat it
as supplied input, not automatic receiver/profile activation. Complete the
remaining target evidence set:

- Supplied: MHI2Q_ER_AUG22_P5092 / MU1329; media driver
  DEV_MMX2QC_AU_ER_G22_201PROD-1; Virtual Cockpit Classic and Extended Map.
  Additional actual binary/API and measured geometry evidence remains open.
- Supplied: Galaxy S23 Ultra and original AAWireless cs307-e2-prod / 3.5.0;
  Android Auto working on MMI. Enhanced cluster/rich-feed behavior is not proven
  by the main screen or another adapter's reports.
- Supplied: Phase 1 NOT INSTALLED. Review the exact original-runtime build/package
  and recovery procedure, then establish the installed original-solution baseline
  through a separately authorized procedure. Preserve stock MMI behavior for
  comparison; record the upstream revision, package identity and recovery results.
  Keep original runtime and reviewed installer-only deviations explicit.
- Exact stock HMI and relevant JCL/DSI/receiver/graphics artifacts or identifiers,
  with provenance and hashes. Keep proprietary binaries and private logs outside
  Git; use the existing `stock/README.md`/`stock_env.sh` asset layout.
- Target build environment for shipping Java 1.4 and QNX/ARM, including actual BSP
  symbol/API compatibility. Import-stub link success is not proof of target ABI.
- Baseline resource/load observations, available storage and measurable timing/
  resource limits for the integrated feature. Define limits from the target,
  not from the host preparation benchmark.

Prepare the existing stock-linked Java tests/build early, before adding target
rendering. Fix only demonstrated linkage issues against this exact profile.

**Files:** existing [target-profile.md](target-profile.md); existing stock/build
instructions and private local assets. A runtime profile, if needed, records
verified identities/capabilities rather than arbitrary operator-entered IDs.

**Acceptance:** every required identity/prerequisite has source evidence; baseline
shipping Java builds against the exact stock API and stock-dependent tests run.
Missing values remain explicit UNKNOWN and block dependent target implementation.

## P2-C03 — display feasibility and ownership gate

**Purpose:** establish an eligible target surface before routing any dashboard.

1. Map the exact firmware's display contexts, displayables, placement, clipping,
   scales and priority/ownership in each supported VC mode. Review
   `common/cluster_surface.*`, `maneuver_render/platform_qnx.c`,
   `ClusterLayerController.java`, existing display/geometry documentation and the
   exact stock DisplayManager implementation.
2. Document a candidate non-critical rectangle and an ownership/restoration table:
   supported mode, rectangle, scale, foreground owner, yield condition, restoration
   destination and behavior if any required fact is unknown.
3. Exercise the real proposed dimensions through host layout/readability checks.
   Current host layouts require at least 640x240, while existing maneuver panels
   can be 328x181 or 210x153. Do not blindly scale the host preview into those panels.
   Redesign Compact/Extended for a genuinely available rectangle if feasible;
   otherwise keep that mode unsupported and suppress output.
4. Choose hosting only from the evidence: reuse an eligible proven graphics path
   and shared primitives; do not assume existing map/maneuver ownership is spare.
   Keep warnings, instrumentation and OEM navigation priorities intact.
5. Define immediate ownership-loss handling and independent OEM restoration when
   the consumer dies. Process cleanup alone is insufficient if HMI context routing
   can remain assigned to a dead renderer.

**Files:** proposed `surface-profile.md`; host layout profiles/tests; later narrow
changes to existing cluster/platform owners only where demonstrated necessary.

**Acceptance / gate:** eligible rectangle, routing, geometry and ownership design
are evidence-backed for every advertised mode; host graphics fit/readability pass;
unknown/unsupported states suppress output. Actual new-renderer conformance is
still tested in C09. If no eligible surface exists, Phase 2 stays BLOCKED; do not
remove the QNX guard, displace guidance or substitute a Phase 3 feature.

## P2-C04 — real source acceptance, capabilities and freshness

**Purpose:** eliminate plausible but unverified live readings.

1. Build a per-signal table: source API/event, meaning, canonical unit, raw unit,
   validity flags, expected update behavior, sample/confirmation age, session and
   route ownership, service-loss behavior and accepted evidence.
2. Verify temperature/range conversions and total-range versus range-1 semantics
   on this diesel. Verify navigation means final-destination distance/duration,
   not next-turn distance; app-specific absent fields remain omitted.
3. Resolve the DSI cache assumption from C02 evidence. Track actual notification/
   confirmation times and subscription health as required by the verified contract.
   Do not renew measurement freshness just because the cache was read. Conversely,
   do not impose an arbitrary timeout that invalidates legitimate change-driven
   values while a proven live subscription says they remain current.
4. Invalidate affected data on service/notification loss, invalid attributes,
   owner/session replacement, route end, clock uncertainty or writer death. A
   writer heartbeat or owner type alone does not prove the underlying source/session.
   Keep independent valid signals available where appropriate.
5. Make production eligibility per signal/profile; never globally turn VEHICLE
   into VERIFIED based on a config flag. Reuse existing availability/provenance
   where sufficient. If DBV1 needs new semantics, introduce an explicit version
   and Java/C compatibility/rejection tests rather than repurpose zero padding.

**Files:** `VehicleDataModule.java`, `AaRgState.java`, `DashboardModule.java`,
`dashboard/vehicle_provider.c`, `dashboard/dashboard.h`, capability/layout policy,
and host/stock-linked source integration tests.

**Acceptance:** every enabled production signal has accepted meaning/units/
validity/lifetime; failures, stale state and session changes cannot revive old
values or produce live zero. Tests cover real source callbacks/service transitions,
not just the existing substituted VehicleDataModule. Unaccepted signals are omitted;
MOCK remains confined to development/test paths.

## P2-C05 — consumer and lifecycle controller on the host

**Purpose:** prepare the missing runtime orchestration without guessing routing.

Implement a small source-independent controller around the existing manager:

- Explicit states for disabled, unsupported target/geometry, waiting for accepted
  data/ownership, active and recovering; every non-active path clears/releases
  output through a backend contract.
- One persistent stateful `db_vehicle_source`, normalized snapshots and manager;
  no implicit mock selection or stateless reader in the production path.
- A bounded acquisition cache outside the draw path. File I/O does not occur in
  render callbacks; dirty/changed data or geometry triggers redraw. Use monotonic
  scheduling and bounded retry/backoff/diagnostics, not an unbounded render loop.
- One shared typed settings contract matching Java/C parsing, plus an idempotent
  configuration/session reconciler. Plan startup, reconnect, disable, restart and
  stale writer handling together rather than adding them as separate patches.
- A fake output backend for deterministic tests of acquire, draw, clear, release,
  failure and ownership transitions. Host profile dimensions are explicit test
  inputs and never a target certification.

**Files:** proposed `dashboard/runtime.c`/runtime header and focused controller
suite; existing manager/provider/config code. Keep QNX compilation blocked here.

**Acceptance:** end-to-end host fixtures run record -> provider -> manager -> frame
-> backend, with disable/failure/unsupported/empty/session transitions clearing
output; bounded recovery, no late publications, no live mock fallback and stable
resource ownership. Repeat source/clock/race failures deterministically.

## P2-C06 — native drawing and VC lifecycle

**Purpose:** supply the actual missing on-unit dashboard backend.

After C02-C05 gates, implement the target-specific adapter (proposed
`dashboard/render_gles.c` and platform adapter):

1. Translate validated `DB_DRAW_RECT`/`DB_DRAW_TEXT` commands with correct
   coordinate origin, clipping, palette, target font metrics and required licensed
   assets. SVG/browser fonts do not supply native text rendering. Preserve the
   documented character policy; any extension needs explicit tests and assets.
2. Use proven Screen/EGL/GLES primitives and bounded resources; allocate/cache
   drawing resources during initialization and reclaim them on stop/context loss.
3. Connect the C05 controller, accepted C04 data and C03 profile to the chosen
   owner. No stock window/critical rectangle may be claimed by guessing an ID.
4. Handle disown, context switch, resize, graphics failure, process crash and
   restart. Clear/release before returning OEM ownership; independently detect
   consumer death so a dead process cannot leave the HMI routed to stale output.
5. Replace the unconditional host-only compilation guard only in the reviewed
   target-support increment, after the gates. Keep unsupported profiles disabled
   and production defaults OFF. A build define alone is not target validation.

**Acceptance:** native drawing/geometry/asset/lifecycle tests pass; exact-target
cross-build links against verified interfaces; no allocations/leaks grow across
repeated transitions. Fake/offscreen tests prove controller behavior only; the
actual VC composition and crash restoration still require C09.

## P2-C07 — controls, configuration and persistence

**Purpose:** make the dashboard usable without sacrificing lifecycle safety.

Use the existing GEM menu generator for Enable, Disable, Compact, Extended and
Restore Default, after the safely gated consumer exists. Do not repurpose wheel
or safety-critical keys. Unsupported target/profile must refuse activation.

- Persist canonical settings atomically in the existing config path. Layout
  selection preserves enablement; restore-default produces disabled Compact.
  Invalid/partial/unknown-version configuration fails closed.
- Keep Java and C settings parsing/versions aligned. Java currently exposes only
  startup enablement; add the needed typed settings/reconciliation instead of
  treating a successful parse as an applied runtime selection.
- Apply settings during the current supported session through serialized lifecycle
  work. Start/stop AA Kombi-only binding when required; retain CarPlay behavior and
  avoid starting all modules or performing bulk I/O under the small state lock.
- Disable immediately suppresses output on the next configured controller cycle
  and triggers OEM restoration; demonstrate measured latency within the agreed
  target budget. A reconnect must not be required to disable or change layouts.
- Restore state after a normal reboot only for valid configuration plus a supported
  target/profile; an old enabled file must not bypass any compatibility/surface gate.

**Files:** Java dashboard config/module and narrow CarPlayApp/VehicleDataModule
lifecycle integration, `dashboard/config.c`/controller, `scripts/build_gem_menu.sh`,
`deploy/gem_menu/run.sh`, menu/CLI/settings/lifecycle tests.

**Acceptance:** every control works through the actual integration; defaults,
invalid config, toggles, selection, reconnect, restart and persistence pass host/
stock-linked tests; applied state is distinguishable from persisted intent.
On-unit control behavior is confirmed in C09.

## P2-C08 — shipping build, packaging and recovery

**Purpose:** turn integrated source into a reproducible reviewable target package.

1. Build the complete shipping Java 1.4 patch against exact stock HMI/JCL where
   applicable, check class-file/API linkage, and cross-build the native consumer.
   A host JDK fixture pass or BSP import-stub link is not sufficient.
2. Register dashboard executable/library/assets/startup files explicitly in build
   staging, installer mapping and uninstall ownership lists. Include only the
   runtime paths; do not stage host mock preview/config tools as the product.
3. Pin firmware/profile, build SHA, protocol/config versions, dependency provenance
   and checksums in the package. Use a distinct fork build identity so it cannot
   be mistaken for the untouched upstream 1.4.1 release.
4. Extend existing fault/upgrade fixtures for new assets: fresh install defaults,
   supported upgrade, unsupported profile/configuration, missing resources,
   insufficient staging space, interrupted activation, recovery and both uninstall
   paths. Maintain conflict backups and the persistent journal protections.
5. Define rollback order: suppress dashboard, stop its consumer/ownership watchdog,
   restore OEM composition, then restore/remove only owned assets/configuration
   according to the documented recovery path. Preserve unrelated customizations.
6. Validate target filesystem semantics, permissions, storage budget and recovery
   access through an authorized controlled procedure. Host SIGKILL fixtures do not
   establish physical power-loss behavior. Do not require destructive power-cut
   experiments on the user's vehicle to satisfy a software test checklist.

**Files:** `scripts/build_{all,renderers,java,release,gem_menu}.sh` as needed,
installer/uninstaller/startup integration, release/validation documents and tests.

**Acceptance:** clean build produces a complete identified package from the pinned
source; missing/mismatched inputs fail visibly; source/host/stock-linked checks
pass; installation/recovery fixtures preserve prior state. Package production
and release publication are separate actions; this plan does not publish a release.

## P2-C09 — actual vehicle acceptance and closeout

**Purpose:** demonstrate the Phase 2 outcome on the actual target.

Use the documented baseline, identified package and a separately authorized
stationary/controlled vehicle procedure performed by the user/qualified operator.
Validate progressively: baseline/default OFF; a permitted real signal in Compact;
Extended with accepted available fields; mode/ownership transitions; then fault
and recovery observations within the approved procedure.

Required matrix:

- Both layouts in every advertised VC mode; real font/readability, clipping and
  unambiguous units. No speed, warnings or OEM guidance obscured.
- Compare accepted real data with OEM/source observations; unavailable inputs and
  route/app changes omit/invalidate the correct widgets without mock replacement.
- Active-session controls, corrupt/deleted configuration, restart/reboot persistence
  and restore-default. Measure applied disable/restoration latency.
- Cold/warm start, phone disconnected, connect/replug, ownership replacement,
  native navigation takeover, reverse/parking views and supported popup transitions.
- Consumer/publisher loss and safe recovery, including an independent OEM fallback;
  validate process failure on a compatible controlled setup before vehicle use.
- Inherited v1.4.1: Android Auto at startup, beyond the two-minute boundary, route
  active at connect, MMI screen switches and documented decoder fallback settings.
- Sustained CPU/RAM/GPU/storage/log activity and repeated transition/soak behavior
  against C02 limits; no growing resource leak or loss of OEM responsiveness.
- Disable, reinstall/recovery and uninstall/rollback restore the recorded baseline.

**Acceptance:** revision/profile-linked results, repeatable passes, measured
budgets, resolved failures and an explicit supported/unsupported mode/signal list.
Keep raw proprietary/private data out of Git. No host-only evidence is relabelled
as hardware acceptance. Only after this matrix passes is Phase 2 GO for closure.

## Audit coverage and exit checklist

| Outstanding point | Owning work packages |
| --- | --- |
| Actual VC renderer, scaling, fonts, resource lifecycle | C03, C05, C06, C09 |
| MMI controls, active-session changes, persistence | C05, C07, C09 |
| Firmware/ABI and exact shipping Java/QNX build | C02, C06, C08 |
| Real values, units, freshness and service loss (T05) | C02, C04, C05, C09 |
| OEM coexistence, crash restoration, reboot/reconnect | C03, C05-C07, C09 |
| Target resource use and latency | C02 limits; C05/C06 implementation; C09 measurements |
| Deployment/recovery/rollback on target | C08, C09; preserve already-tested installer fixes |
| Remote CI evidence | C01 and final relevant implementation revision |
| v1.4.1 target behavior (T07) | C01 behavioral protection, C08 package, C09 hardware |

- [ ] CI result observed for the relevant implementation revision.
- [x] User-supplied firmware/MU/media driver, VC modes, adapter and Phase 1 status recorded.
- [ ] Exact stock/build prerequisites and actual Phase 1 original-solution baseline established.
- [ ] Supported surface/ownership and signal policies accepted with evidence.
- [ ] Controller/native backend and runtime settings integrated and tested.
- [ ] Complete target package and recovery path validated.
- [ ] Actual vehicle matrix and resource/latency limits pass.
- [ ] Scope/support/limitations and Phase 2 GO decision recorded for that revision.

**Recommended next implementation:** start C01 diagnosis and C05's pure host
controller/settings contract; complete the C02 evidence inventory and C03 source
analysis alongside them. Do not wait for all hardware facts to begin independent
host work, and do not begin target routing while those facts remain unknown.
