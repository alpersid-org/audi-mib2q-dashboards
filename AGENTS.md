# Instructions for AI agents

These instructions apply throughout this repository. Agents that do not discover
AGENTS.md automatically should read it explicitly. Follow the current user's
instructions and the agent platform's higher-priority rules; use these project
instructions to preserve context, not to override them.

## Goal and current scope

Build the foundation for modular, customizable Audi Virtual Cockpit dashboards
within **roadmap Phase 2** of the mib2q-carplay-based project.

**Current task is P2-04:** the user requested "Start P2-04, but before make sure you update the documentation, as instructed".
P2-04 is PLAN step 4 (automated validation, benchmark, regression report) and is
the final testing stage the earlier no-test instruction waited for, so host tests
are now authorized. Work on `feature/p2-04-validation`. No GEM, master merge,
release, vehicle or Android work. P2-03 (config/manager) is complete on master
but was untested until P2-04; see [config.md](docs/custom-dashboards/config.md).

**Completed earlier — PUBLISH-MASTER:** the user explicitly requested "Push everything
to main/master on my forked project". All Phase 2 work through P2-02, roadmap and
agent documents, and MAINT-141 verification have been fast-forwarded and pushed
to the fork's `master`. See the audit checkpoint for publication evidence.
This supersedes the earlier local-only maintenance restriction for this push.
No release, vehicle action, further feature or Android implementation is authorized.
The v1.4.1 release was already inherited; avoid duplicate cherry-picks/rewrites.

On each resume, reconcile this scope with the
latest explicit user instruction and the scope recorded in
[audit/CURRENT.md](docs/custom-dashboards/audit/CURRENT.md). Preserve an already
authorized host task across agent switches; update stale scope summaries when
implementation is actually requested. The audit is evidence of work and scope,
not permission to bypass safety rules or higher-priority instructions.

Act as an embedded engineer familiar with Audi MIB2/MHI2Q, QNX, C/C++, Java,
graphics and Android Auto. Prefer demonstrated repository behavior over general
platform assumptions. Do not invent firmware capabilities or compatibility.

## Read before choosing work

Start with [the active audit checkpoint](docs/custom-dashboards/audit/CURRENT.md)
and recent [audit events](docs/custom-dashboards/audit/LOG.md). Follow
[the audit protocol](docs/custom-dashboards/audit/README.md) before editing.

1. [Five-phase roadmap](docs/ROADMAP.md) — long-term goal and phase boundaries;
   distinguish roadmap Phase 3 from Phase 2 subtask P2-03.
2. [PROJECT.md](PROJECT.md) — project entry point and intended environment.
3. [Status](docs/custom-dashboards/status.md) — actual completed work and blockers.
4. [Requirements](docs/custom-dashboards/requirements.md) — scope and safety rules.
5. [Investigation](docs/custom-dashboards/analysis.md) — evidence and source links.
6. [Architecture](docs/custom-dashboards/architecture.md) and
   [provider design](docs/custom-dashboards/data-providers.md) — shared contracts
   and explicit gaps between implemented code and future adapters.
7. [Executable plan](docs/custom-dashboards/PLAN.md) — Phase 2 work and acceptance.
8. [Handoff](docs/custom-dashboards/HANDOFF.md) — restart checklist and next task.

Read the original README and relevant source files before changing a subsystem.
The original [docs/architecture.md](docs/architecture.md) describes upstream;
[docs/custom-dashboards/architecture.md](docs/custom-dashboards/architecture.md)
describes the proposed dashboard increment. Do not confuse the two.

## Repository and branch discipline

- Origin: `https://github.com/alpersid-org/audi-mib2q-dashboards.git`.
- Upstream: `https://github.com/kamgurgul/mib2q-carplay.git`.
- Consolidated published branch: `master`, tracking `origin/master`. The user
  authorized the 2026-11-07 fast-forward of feature and maintenance work into it.
  Retain `feature/custom-dashboards` and `maintenance/upstream-v1.4.1` as history.
  Use a dedicated branch for future implementation; do not infer approval for
  subsequent master merges, pushes or releases from this completed publication.
- Investigated upstream baseline: `b127988a68c8d690841f4a75d35717ffc01e668e`.
  The fork inherits the actual upstream source and Git history, not a submodule.
- Inspect status, branch, remotes and recent commits first. Preserve existing
  user edits. Do not reset, force-push or rewrite other people's commits.
- Push project work to the fork, never upstream. Publish only within the user's
  authorized scope; do not merge, release or change repository settings implicitly.
- Keep dashboard documents under `docs/custom-dashboards/`. Preserve upstream
  implementation and README unless a justified, authorized change needs them.
- Do not commit credentials, proprietary HMI jars/JXE, vehicle logs, firmware
  extracts, local toolchains or generated binaries. Preserve upstream licenses.

## Vehicle facts and unknowns

The supplied target is an Audi A4 B9 (2017), 2.0 TDI quattro 190 HP, MMI High
without touchscreen, Virtual Cockpit and physical MMI rotary controller/buttons.
The phone is a Samsung Galaxy S23 Ultra using Android Auto through the **original
AAWireless adapter, not AAWireless TWO+**.

Exact MHI2Q firmware, hardware/receiver ABI, original adapter compatibility,
cluster geometry and successful Phase 1 installation remain unconfirmed.
Reports for another car, firmware or TWO+ do not establish this combination.
Never turn successful host compilation into a claim of vehicle compatibility.

## Safety boundary

- Work on the host. Do not connect to, execute commands on, install, flash or
  deploy to the head unit/vehicle under this project's current scope.
- Never change vehicle control, braking, steering, ADAS, warning logic or other
  critical systems. Never replace or obscure speed or warning instrumentation.
- Preserve OEM functionality and navigation guidance. Do not intercept or
  repurpose safety-critical controls. Do not add dashboard key interception.
- Do not destructively modify stock files, enable ABI trial modes, guess native
  offsets, or introduce invasive/unproven hooks to obtain a dashboard surface.
- A free displayable ID or phone SafeArea is not proof of a safe VC rectangle.
  Existing displayables 98 and 99 serve maneuver guidance and map video; their
  coexistence in context 82 does not prove room for extra dashboard graphics.
- Keep future dashboard functionality disabled by default. Missing/invalid config,
  failed providers and unsupported geometry must suppress output safely.
- If safe rendering cannot be established, retain a standalone host prototype.
  Report the integration blocker instead of forcing a vehicle implementation.
- Do not execute deploy/mod scripts directly as a shortcut. Inspect any test
  script before running it; installer tests must use isolated host fixtures.

During Phase 2 do not implement roadmap Phases 3–5: no Android companion app,
new Android-to-MMI transport or display of companion-app data. Documenting their
requirements and designing stable shared provider boundaries is mandatory.
Phase 3's standalone app must initially run independently on the Galaxy S23 Ultra.
VehicleDataProvider and future AndroidDataProvider must eventually feed the same
Dashboard Manager, normalized snapshots and widgets. No DPF monitoring.

## Evidence and implementation policy

For important conclusions, state **confirmed by existing code**, **technically
plausible but unverified**, or **unsupported/unknown**, and cite source paths.
Separate upstream reports, host test evidence and validation on this actual car.
Recheck evidence at the current revision before relying on old notes.

When host implementation is requested, follow the next eligible PLAN step:

- Prefer a small isolated C99 module, existing abstractions and bounded resources;
  avoid unnecessary frameworks, dependencies and rendering threads.
- Proposed slice: typed snapshots and mock provider, widget formatting, Compact
  and Extended layouts, config/suppression, host adapter, tests and previews.
- Treat the architecture as a proposal. Explain justified design adjustments in
  the docs instead of silently drifting or treating choices as proven requirements.
- Initially mark every sample as MOCK. Connect a real field only after its source,
  meaning, units, validity, freshness and session ownership are verified.
- VehicleDataModule is CarPlay-only in the investigated baseline; do not assume
  it supplies vehicle snapshots during Android Auto. Rich navigation metadata is
  app dependent; next-turn distance is not final-destination distance.
- Design provider snapshots independently of phone transport, with availability,
  provenance and sample age. Missing/stale data must not appear as live zero.
- Prototype dimensions and host font rendering are not certified VC support.
  Validate readability and reject unsupported crops rather than guessing a fit.
- Add GEM activation/config controls only after a safe consumer is established.
  Upstream menu generation alone is insufficient evidence for dashboard activation.

## Validation and handoff

**MAINT-141 exception:** the latest user request explicitly authorizes all
applicable automated maintenance/dashboard tests, including tee/SIGPIPE tests.
Run those host checks and report missing assets/toolchains separately. This
supersedes the older deferral below for this task only; it does not authorize
vehicle execution, publication or unrelated feature development.

**P2-04 final testing stage (current):** "Start P2-04, but before make sure you update the documentation, as instructed"
ends the P2-02 override below. Run the dashboard suites, sanitizers, the host
benchmark and the inherited host scripts listed in PLAN step 4 (inspect each
first; isolated host fixtures only). Record exact commands and outcomes; report
missing assets/toolchains as unavailable, never as passes. Vehicle execution,
publication to master and releases remain unauthorized.

**User override during P2-02 (historical, ended by P2-04):** "Sa nu executi teste momentan, testele sunt
pentru final, mergem inainte fara teste. Doar sa documentezi fiecare schimbare."
Do not execute further unit, integration, sanitizer, browser/visual or automated
validation tests until the user requests the final testing stage. Continue
implementation and document every change. Tests already run before this request
remain historical evidence, not permission to rerun them. Record deferred checks
explicitly; do not claim final validation. This overrides the run-test guidance
below while the instruction is active, including during subsequent agent handoffs.

For documentation changes, check relative links, Markdown code fences, Git diff
whitespace and change scope. Do not run full firmware builds for a text edit.

For future implementation, run meaningful widget, mock-provider, config, layout,
rendering, fallback and bounded performance checks, plus appropriate inherited
regression scripts listed in PLAN.md. Inspect prerequisites first. P2-01 uses
`make -C dashboard all test demo` and `make -C dashboard test-sanitize`.
Root `make`/preview/benchmark targets in later PLAN steps do not yet exist.

The recorded upstream baseline is not a clean test pass: LeakSanitizer was
blocked by tracing restrictions; a retry with leak detection disabled later
failed `bus_transport_test` at line 84. Its cause is unresolved. Do not silently
disable sanitizers, relabel failures as skips or assume a sandbox cause. Record
exact commands, environment adjustments, results and missing prerequisites.
P2-01 later passed the full C suite outside the traced sandbox with the documented
`SKIP_JAR_CHECK=1` option (shipping JAR absent). Bus transport passed there; do not
claim its old failure proves a code defect or that missing-JAR checks passed.
Java/QNX/macOS-specific checks may need assets/toolchains not supplied here.

## Mandatory agent audit trail

Every agent must use `docs/custom-dashboards/audit/`, including Claude resuming
Codex work. Append a START/RESUME event, record the active task and authorized
scope, and reconcile the checkpoint with the real Git tree before editing.
Update CURRENT.md and append a factual event at meaningful milestones/test
batches/blockers, before long or disruptive operations, at least every 30 minutes
of active work, and before stopping, compaction or a model handoff. Reserve
context for checkpointing; do not wait for token exhaustion.

Record partial files, dirty/untracked work, exact commands/results, pending
processes, decisions with evidence, blockers and the exact next action. Keep
LOG.md append-only; correct history with a new event. Do not store secrets or
private reasoning. Save source changes and checkpoint together; commit/push when
authorized so another machine can retrieve them. Mark WIP and unverified checks
honestly. If a previous agent stopped abruptly, append a recovery RESUME event
after comparing the checkpoint with actual files and commits.

Before ending a work increment:

1. Check the final diff for unrelated changes and forbidden artifacts.
2. Update status and the plan with actual completed acceptance criteria, test
   evidence, remaining blockers and the next eligible task.
   Update audit/CURRENT.md and append a COMPLETE or HANDOFF/PAUSE event too.
3. Keep the handoff accurate when scope or architecture changes; do not copy
   stale permission restrictions over a later explicit user instruction.
4. Commit/push when requested and verify the remote branch matches the local
   commit. Report publication failures truthfully.
5. Report what changed, validation, material limitations and the next step.
   Host results never prove vehicle safety.
