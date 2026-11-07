# Executable Phase 2 plan

This plan subdivides **roadmap Phase 2 only**. Later roadmap phases are excluded.
Run commands from this project's root (the directory containing PROJECT.md).
Steps marked blocked require new evidence;
they are not authorization to contact, install on or deploy to the vehicle.

**Current authorized scope:** complete step 0 and publish documentation to Git.
Steps 1–5 are a future executable implementation plan; their commands become
available when the corresponding source, Makefile and tests are implemented.
Step 6 remains blocked by vehicle-specific evidence. No implementation code or
generated previews are included in the documentation release.

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
| 1 | 0; separate implementation request | Host module, typed snapshot contract, mock provider and widget formatters | Deferred |
| 2 | 1 | Compact/Extended frame builders and SVG adapter | Deferred |
| 3 | 1–2 | Disabled-by-default configuration and suppression behavior | Deferred |
| 4 | 1–3 | Automated tests, host benchmark and regression report | Deferred |
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

## 2. Implement the two layouts and host renderer

1. Prepare Compact 2x2 and Extended 3x2 grids from an explicit viewport.
2. Bound every primitive and clip long text; use a dark, high-contrast theme.
3. Export SVG through a host adapter. No renderer sockets or display windows.
4. Generate wide 800x252 and tall 640x360 host previews; reject undersized crops.

Execution after implementation:

```sh
make
make previews
```

Acceptance: both layouts show clear MOCK labels and a host-only banner; exported
graphics stay inside the requested rectangle. Dimensions do not claim VC support.

## 3. Configuration and safe suppression

1. Add strict versioned configuration with disabled Compact defaults.
2. Make CLI layout selection independent of enablement.
3. Implement host disable and restore-default actions.
4. Clear old frames on invalid config/viewport or provider failure.

Acceptance: no config and malformed config emit no dashboard drawing commands;
switching layouts does not silently activate the feature. No GEM code is changed.

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
