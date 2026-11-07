# Executable Phase 2 plan

## Current final-stage acceptance

Available host checks and the strengthened shutdown regression pass; see
[validation-final.md](validation-final.md). R4, obsolete widget suites and the
maintenance preservation check are repaired; all four SVG previews are present.
Stock Java/DSI/QNX and macOS GL prerequisites remain unavailable. Commit/push to
the fork master is authorized. Steps 6/9/10 are not completed: firmware/ABI, safe
surface and actual vehicle validation remain mandatory. No unproven hook or
head-unit action is authorized.

## Review gate at 312215a (2026-11-07)

[Manual review](reviews/claude-312215a.md) records five open findings. Before
connecting the step-9 consumer, resolve producer/reader clock ordering and writer
shutdown/session invalidation. Repair/execute tests only at the user-requested
final stage. This review did not start implementation or authorize publication.

MAINT-141 is completed upstream maintenance verification, not a plan step or new
phase. Applicable maintenance/dashboard tests were authorized; see
[the report](../maintenance/upstream-v1.4.1.md). The subsequent user request to push
everything to the fork's main/master authorized consolidation into `master`.
P2-01/P2-02, roadmap and maintenance work are now included there. No new features
or tests were added for publication. Historical feature-test deferral below is
superseded only for maintenance scope.

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

**P2-05 (current):** user request "Start P2-05". Packaging only, on
`feature/p2-05-package`: [package.md](package.md). No tests or generation run;
inherited-file check done by read-only git (all 48 upstream diffs are additions).

**P2-04 (complete, historical):** user request "Start P2-04, but before make sure you update the documentation, as instructed".
This is the final testing stage; the testing override below has ended. Done on
`feature/p2-04-validation`; report in [validation.md](validation.md). Targets are
`make -C dashboard test|test-sanitize|benchmark` (no root Makefile; see report).
Not merged to master.

**P2-03 (complete, merged to master on request):** user request "Start P2-03 on a dedicated feature branch".
Implemented on `feature/p2-03-config-manager`; see [config.md](config.md).
Compiled only; tests remain deferred to the final stage. Not merged to master.

**Scope update (authoritative):** Phase 2 is "Real Vehicle Data Integration";
see [requirements.md](requirements.md#phase-2-scope--real-vehicle-data-integration-authoritative).
Steps 1–5 below built a mock-only host foundation and are complete; they do not
complete Phase 2. The remaining work is the expanded step 6 and the proposed
steps 7–10 at the end of this plan. Steps that run on the head unit need the
user's explicit change to the safety boundary first.

This plan subdivides **roadmap Phase 2 only**. Later-phase implementation is
excluded; designing compatible provider boundaries for those phases is required.
Read the complete [five-phase roadmap](../ROADMAP.md) and
[provider design](data-providers.md). P2-03 configuration is not roadmap Phase 3's
standalone Android app.
Run commands from this project's root (the directory containing PROJECT.md).
Steps marked blocked require new evidence;
they are not authorization to contact, install on or deploy to the vehicle.

**Current authorized scope:** P2-02 / step 2 on the host. User instruction:
"Okay, push, commit and let's go with step 2." P2-01 is complete; step 2 now
supplies Compact/Extended frames and SVG previews. Steps 3–5 are later work;
vehicle step 6 remains blocked.

**Testing override (historical, ended by P2-04):** the user subsequently instructed that no further tests
should run now; testing is for the final stage. Continue implementation and
document every change. Test commands below are retained for that final stage,
not permission to execute them now. Earlier results remain in the audit.

Current documentation update: DOC-ROADMAP defines the complete roadmap and common
VehicleDataProvider/AndroidDataProvider architecture. It does not start P2-03,
implement a live provider or implement any Android communication.

Actual step 1 commands: `make -C dashboard all test demo` and
`make -C dashboard test-sanitize`; see [module.md](module.md). The bare `make`
commands in later steps describe targets to implement, not available root targets.

## 0. Create the project and document decisions

1. Retain the full source and Git history of `kamgurgul/mib2q-carplay` in this
   project, starting at `b127988a68c8d690841f4a75d35717ffc01e668e`.
2. Create `feature/custom-dashboards`; keep the original implementation unchanged
   and put our planning documents under `docs/custom-dashboards/`.
3. Write PROJECT.md, architecture, this plan and the source-backed investigation
   before any future implementation in this project.
4. Add requirements, a safety boundary and a truthful status record.
5. Commit and push the documentation-only project to its own Git repository.
6. Maintain root AGENTS.md and an AI continuation guide; keep tool-specific entry
   files as pointers to the canonical instructions.
7. Maintain `audit/CURRENT.md` and append-only `audit/LOG.md`; require agents to
   checkpoint milestones, tests and incomplete work before context/model handoff.

Acceptance: documentation identifies controllable surfaces, source limitations,
AAWireless model uncertainty and the host-only fallback. The branch contains
upstream's history/source and changes only documentation.

Verification:

```sh
git branch --show-current
git merge-base --is-ancestor b127988a68c8d690841f4a75d35717ffc01e668e HEAD
git diff --name-only b127988a68c8d690841f4a75d35717ffc01e668e HEAD
```

## Work order and dependencies

| Step | Depends on | Deliverable | Current state |
| --- | --- | --- | --- |
| 0 | Repository investigation | Full upstream-derived project plus Markdown documents and Git publication | Documentation complete; destination fork verified as alpersid-org/audi-mib2q-dashboards |
| 1 | 0; user request received | Host module, typed snapshot contract, mock provider and widget formatters | Implemented; focused host tests pass; see module.md and audit |
| 2 | 1; user request received | Compact/Extended frame builders and SVG adapter | Implemented with four saved host SVGs; final testing/visual review deferred by user |
| 3 | 1–2; user request received | Disabled-by-default configuration and suppression behavior | Implemented on host (config.c/manager.c, two CLIs); compiled, tests deferred by user |
| 4 | 1–3; user request received | Automated tests, host benchmark and regression report | Complete on `feature/p2-04-validation`; dashboard suites/sanitizers pass; upstream results and unavailable checks in [validation.md](validation.md) |
| 5 | 4; user request received | Reproducible previews, documentation and rollback evidence | Documented in [package.md](package.md) on `feature/p2-05-package`; nothing re-executed (no-test instruction) |
| 6 | 5 plus firmware/surface evidence | Signal and surface feasibility (scope req. 1–2) | Source review partial; on-car evidence and head-unit authorization needed |
| 7 | 1–5; user request received | Remove unverified widgets and keep mock out of product paths (req. 6, 8) | Implemented on `feature/p2-07-verified-widgets` ([widgets-p2-07.md](widgets-p2-07.md)); compiled; tests out of date, deferred by user |
| 8 | 6–7; user request received | VehicleDataProvider: real-time binding through existing pipelines (req. 3–4) | Implemented on master (merged on approval) ([vehicle-provider.md](vehicle-provider.md)); C compiled, most Java uncompilable here (stock classes); not run; verification needs the car |
| 9 | 6, 8 | Virtual Cockpit renderer for Compact/Extended (req. 5) | Proposed; needs surface decision and head-unit authorization |
| 10 | 8–9 | In-car validation by the user (expected outcome) | Proposed; user installs and tests |

Each future step should be a reviewable commit. Fix a failed acceptance criterion
before starting dependent work; record unavailable checks with their actual cause.
Use task IDs `P2-01` through `P2-06` for steps 1–6 in the audit trail; record
subtasks under the same feature ID. Read the active checkpoint before resuming.

## 1. Establish the isolated module and provider

1. Define fixed-capacity snapshot and frame types in `dashboard/dashboard.h`.
2. Add deterministic mock snapshots with explicit source and sample age.
3. Add widget formatters using documented canonical units.
4. Reject missing, invalid, future-dated and stale fields.

Acceptance: host C99 compilation needs no QNX SDK, stock HMI jar or new package;
all candidate values are mock, with no connection to vehicle or phone data.

P2-01 acceptance completed: bounded typed snapshots and frame command types,
deterministic mock callback, formatter units/provenance, validity/freshness guards,
provider failure clearing and focused tests. Commands and constraints are in
[module.md](module.md); results are in [status.md](status.md). No layouts,
renderer or configuration manager are claimed by this increment.

## 2. Implement the two layouts and host renderer

1. Prepare Compact 2x2 and Extended 3x2 grids from an explicit viewport.
2. Bound every primitive and clip long text; use a dark, high-contrast theme.
3. Export SVG through a host adapter. No renderer sockets or display windows.
4. Generate wide 800x252 and tall 640x360 host previews; reject undersized crops.

Execution after implementation:

```sh
make -C dashboard all
make -C dashboard previews
```

Acceptance targets: both layouts show clear MOCK labels and a host-only banner;
exported graphics stay inside the requested rectangle. Dimensions do not claim
VC support. Implementation and artifacts exist; final visual/testing acceptance
is deferred by user instruction. See [rendering.md](rendering.md) and
[changes.md](changes.md).

## 3. Configuration and safe suppression

1. Add strict versioned configuration with disabled Compact defaults.
2. Make CLI layout selection independent of enablement.
3. Implement host disable and restore-default actions.
4. Clear old frames on invalid config/viewport or provider failure.
5. Keep the manager's input behind the generic `db_provider` boundary. Vehicle
   and future Android adapters must share that path; the host preview may remain
   explicitly mock-only. Do not hard-code mock acquisition into widgets/manager.
   Multi-provider resolution and origin metadata follow the provider design when
   needed; no Android transport or live vehicle subscription is part of P2-03.

Acceptance: no config and malformed config emit no dashboard drawing commands;
switching layouts does not silently activate the feature. No GEM code is changed.

P2-03 implementation state: items 1–5 implemented in
[config.c](../../dashboard/config.c), [manager.c](../../dashboard/manager.c),
`dashboard_config` and `dashboard_manager_preview`; the manager takes any
`db_provider` and the mock is chosen only by the CLI. No GEM code changed.
Acceptance is by design and strict compilation only; the listed final-stage
checks in [config.md](config.md) are deferred by user instruction.

## 4. Automated validation and performance evidence

```sh
make test
make test-sanitize
make benchmark
```

Cover rendering semantics, widget units and bounds, mock determinism, configuration,
layout compatibility, clearing and fallback. Parse SVG artifacts independently.
Use a bounded host preparation benchmark; do not infer head-unit CPU/FPS from it.

Run existing inherited upstream scripts from this project's root:

```sh
sh scripts/run_tests.sh
bash scripts/test_route_info.sh
bash scripts/test_java_transports.sh
bash scripts/test_maneuver_native.sh
bash scripts/test_mmi_settings.sh
bash scripts/test_pdc.sh
bash scripts/test_lane_guidance_e2e.sh
bash scripts/audit_maneuvers.sh
bash scripts/audit_java_stock.sh
```

Acceptance: new host suite passes. Record actual upstream passes/failures/skips,
including sandbox limitations, missing proprietary HMI assets, JDK/QNX toolchains
and the macOS-only GL link path. Do not download proprietary firmware or change
production code to make unavailable tests appear to pass.

## 5. Package the reviewable increment

1. Save Compact/Extended previews in `dashboard/previews/`.
2. Document exact reproduction commands, test outcomes and remaining limitations.
3. Summarize changed files and rollback.
4. Verify inherited implementation files remain unchanged relative to the
   investigated upstream commit.

Acceptance: a developer can clone the fork, build, test and inspect the
prototype without vehicle access; documentation makes the integration blocker clear.

P2-05 state: items 1–4 documented in [package.md](package.md). Previews are
the P2-02 files verified in P2-04 (no code change since). Item 4 verified by
`git diff --name-status b127988 f7e9fb2`: 48 additions, no modifications or
deletions. The clone/build/test walkthrough is documented, not re-executed,
under the user's no-test instruction.

## 6. Vehicle feasibility investigation — BLOCKED / future Phase 2 work

Inputs still needed before proposing any on-car test:

- Independently confirmed firmware train/MU, hardware/cluster variant and Phase 1
  behavior, obtained through a separately authorized process.
- Exact HMI/receiver ABI evidence and original AAWireless behavior. TWO+ reports
  cannot be substituted.
- Evidence for an available non-critical graphics rectangle in each intended
  VC layout, with no speed/warning/navigation coverage.
- Reviewed ownership, suppression, disconnection, crash and OEM restoration rules.
- Verified read-only signal meanings, validity, units and sample lifetime on this
  vehicle, especially range semantics and availability during Android Auto.

Only after these gates could a separate increment implement a renderer adapter
and then GEM controls. If the surface cannot be demonstrated, retain the host
prototype. No invasive hook, vehicle-control change, stock-file destruction,
flashing, installation, deployment or head-unit execution is permitted here.

## 7–10. Real Vehicle Data Integration — proposed (scope update)

Added after the user's Phase 2 scope. Not started; each needs its own request.

7. **Omit unverified widgets (host).** Remove trip distance and average
   consumption from Extended (no source found; req. 8) and redefine Extended
   around verified signals. Keep `db_mock_read` for tests/development only.
   *State:* implemented; see [widgets-p2-07.md](widgets-p2-07.md). Existing
   tests are out of date and the Extended previews were removed; both are for
   the final testing stage.
8. **VehicleDataProvider (req. 3–4).** Reuse existing pipelines: the
   `VehicleDataModule` DSICarKombi readings (outside temperature, range), the
   Android Auto trip feed (final-destination distance/time) and existing
   now-playing events, made available during Android Auto sessions without
   breaking CarPlay. Publish read-only snapshots to the dashboard over the
   existing bus; convert to `db_snapshot` with real provenance, units, validity
   and sample age. Missing signals stay MISSING. No new DSI/CAN queries, no
   vehicle control.
   *State:* implemented; see [vehicle-provider.md](vehicle-provider.md). Media
   is not supplied (no verified AA source in the repo); CarPlay navigation is
   not wired; nothing consumes the record until step 9.
9. **Virtual Cockpit renderer (req. 5).** Port `db_frame` drawing onto the
   existing cluster GLES path, on a surface chosen by the user and validated on
   the car; it must yield to speed, warnings and navigation and stay disabled
   by default.
10. **In-car validation.** The user installs and confirms real values and safe
    composition on this A4. Only then is Phase 2 complete.
