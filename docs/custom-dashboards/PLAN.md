# Executable Phase 2 plan

MAINT-141 is completed upstream maintenance verification, not a plan step or new
phase. Applicable maintenance/dashboard tests were authorized; see
[the report](../maintenance/upstream-v1.4.1.md). The subsequent user request to push
everything to the fork's main/master authorized consolidation into `master`.
P2-01/P2-02, roadmap and maintenance work are now included there. No new features
or tests were added for publication. Historical feature-test deferral below is
superseded only for maintenance scope.

**P2-04 (current):** user request "Start P2-04, but before make sure you update the documentation, as instructed".
This is the final testing stage; the testing override below has ended. Work on
`feature/p2-04-validation`. Not merged to master.

**P2-03 (complete, merged to master on request):** user request "Start P2-03 on a dedicated feature branch".
Implemented on `feature/p2-03-config-manager`; see [config.md](config.md).
Compiled only; tests remain deferred to the final stage. Not merged to master.

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
| 4 | 1–3; user request received | Automated tests, host benchmark and regression report | In progress on `feature/p2-04-validation` (final testing stage) |
| 5 | 4 | Reproducible previews, documentation and rollback evidence | Deferred |
| 6 | 5 plus firmware/surface evidence | Separately reviewed vehicle feasibility decision | Blocked; no vehicle execution authorized |

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
