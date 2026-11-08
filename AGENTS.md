# Instructions for AI agents

These instructions apply throughout this repository. Agents that do not discover
AGENTS.md automatically should read it explicitly. Follow the current user's
instructions and the agent platform's higher-priority rules; use these project
instructions to preserve context, not to override them.

## Goal and current scope

**Latest — P2-C07 host part (merged):** settings persistence and live
application ([controls.md](docs/custom-dashboards/controls.md)). GEM controls stay
unimplemented until a safely gated consumer exists (C06).

**Earlier — P2-C01:** GitHub Actions enabled; first CI run passed ([ci-status.md](docs/custom-dashboards/ci-status.md)).
CI now runs on every push; check run results before claiming CI status.

**Earlier — P2-C05 settings contract (merged):** C05 is complete on the host. the user asked to
"start C05 settings contract". One typed v1 settings contract shared by C and
Java with 48 conformance vectors ([settings-contract.md](docs/custom-dashboards/settings-contract.md)).

**Earlier — P2-C04 host part (merged):** the user asked to "start C04". Signal
acceptance register and per-signal eligibility are implemented and host-tested
([signal-acceptance.md](docs/custom-dashboards/signal-acceptance.md)). Production
mode shows only ACCEPTED signals; none is accepted until vehicle evidence exists.

**Latest — FIX-CLAUDE-C05 (published):** user requested "Fix all findings" from the
[C05 review](docs/custom-dashboards/reviews/claude-p2-c05.md). Shutdown now latches
disabled intent until explicit enable; incomplete backends are rejected before
ownership. Focused regressions and C/sanitizer suites pass. Phase 1 is complete;
stale installation instructions reconciled. See [resolution](docs/custom-dashboards/reviews/claude-p2-c05-resolution.md)
and audit checkpoint. Committed as `62b18e1`, fast-forwarded and pushed to fork
master on "Commit and push to master"; remote SHA verified.

**Earlier — P2-C05 (merged):** the user asked to "start C05"; the host runtime
controller ([runtime.md](docs/custom-dashboards/runtime.md)) is merged to master
with passing host tests. Next eligible host work: C05's shared Java/C settings
contract or C04 signal policy. Native drawing (C06) stays gated on C03 evidence;
no vehicle actions.

**Current — P2-COMPLETION-PLAN:** user requested reviewing the audit and planning
all missing points using best judgment. The current execution sequence is
[completion-plan.md](docs/custom-dashboards/completion-plan.md), C01-C09. This
increment updates planning/handoff documentation; earlier tested fixes remain
published. Execute independent host work within existing authorization, keep
firmware/signal/safe-surface evidence explicit, and do not infer vehicle operations
or target routing from this plan. Prior commit/push authorization persists.


**Current — P2-COMPLETION-AUDIT:** the user requests correcting the independent
audit findings, running applicable tests, then committing/pushing to fork master.
This supersedes the earlier audit-only task. Correct host-code and deployment
safeguards; keep hardware/safe-surface requirements explicit. No vehicle access,
release, guessed routing or Phases 3–5. Read CURRENT.md for active evidence.


**Current — P2-FINAL host work complete:** Codex resumed from b180e36, completed
available inherited regressions and reproduced/fixed an additional R2 blocked-
second-write shutdown defect. Available C/sanitizer/Java/feed/lifecycle/upstream
checks pass; stock Java/DSI/QNX/macOS GL and vehicle gaps remain explicit. Read
[validation-final.md](docs/custom-dashboards/validation-final.md) and CURRENT.md.
The latest user request authorizes committing/pushing this completion to the fork's
master. No release or vehicle action. Step 9 stays blocked pending firmware/ABI
and proven safe-surface evidence; Phase 2 is not a completed vehicle feature.

**Earlier — FIX-REVIEW-312215A (see final validation for later test evidence):** the user asked to "implement all fixes" from the
review. R1, R2, R3, R5, session invalidation and the AA subscription gap are
implemented and untested on `fix/review-claude-312215a`; R4 (test harness) stays
open for the final test request. See the
[resolution record](docs/custom-dashboards/reviews/claude-312215a-resolution.md).
Merged to master.

**Earlier handoff — REVIEW-CLAUDE:** the review of all Claude commits through
`312215a` is complete. Read the [review report](docs/custom-dashboards/reviews/claude-312215a.md)
and [agent pickup instructions](docs/custom-dashboards/reviews/README.md) before
continuing Phase 2. Five findings remain open, including producer/reader timestamp
ordering and writer shutdown. The user has now requested "Push and commit": publish these review documents
on `review/claude-phase2`. No corrective implementation, tests, master merge or
release is part of that request.
Use the live checkpoint and latest user instruction to determine the next task.

**Phase 2 goal (authoritative, user-defined):** "Real Vehicle Data Integration" —
fully functional custom dashboards showing real data from the vehicle's existing
MMI systems inside the Virtual Cockpit, Compact and Extended layouts, mock data
only for tests/development, unverified signals omitted, no dependency on the
future Android app. Read the full text and gap table in
[Phase 2 scope](docs/custom-dashboards/requirements.md#phase-2-scope--real-vehicle-data-integration-authoritative). **Phase 2 is not complete:** P2-01..P2-05 are a
mock-only host foundation. Head-unit work still needs the user's explicit change
to the safety boundary below (open decision recorded in that section).

**Testing (superseded by P2-FINAL below):** after P2-04 the user said "do not run any test, there will be a ask at the end to implement and run the tests".
No tests or new test code until that final request; see Validation below.

**Earlier task — P2-08 (merged; reviewed with open findings):** the user said "merge to master and start step 8".
VehicleDataProvider implemented on `feature/p2-08-vehicle-provider`, including
minimal edits to inherited HMI Java (CarPlayApp, VehicleDataModule, AaRgState,
AaBridge) that the step requires. Writing this code does not authorize
installing or running it on the unit; the user does that in step 10. See
[vehicle-provider.md](docs/custom-dashboards/vehicle-provider.md).

**Earlier task — P2-07 (merged):** omit unverified widgets; see
[widgets-p2-07.md](docs/custom-dashboards/widgets-p2-07.md).

**Earlier update — DOC-SCOPE:** the user supplied the Phase 2 scope above and
asked that the documentation reflect it. Documentation only; no tests.

**Earlier task — P2-05:** the user requested "Start P2-05" (PLAN step 5,
package the reviewable increment) on `feature/p2-05-package`; documentation only,
no tests. See [package.md](docs/custom-dashboards/package.md).

**Earlier task — P2-04 (complete; merged to master on request):** the user requested "Start P2-04, but before make sure you update the documentation, as instructed".
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
Then read the [current review handoff](docs/custom-dashboards/reviews/README.md)
and its linked report; reconcile findings with newer source before applying fixes.

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

Current user-supplied profile: [target-profile.md](docs/custom-dashboards/target-profile.md).
Audi A4 B9/2017, MHI2Q_ER_AUG22_P5092, MU1329, media driver
DEV_MMX2QC_AU_ER_G22_201PROD-1, Virtual Cockpit Classic and Extended Map;
Galaxy S23 Ultra, Android Auto working on MMI, original AAWireless
cs307-e2-prod / firmware 3.5.0. **Phase 1 IS installed and vehicle-tested** (upstream
1.4.1 release; [Phase 1 handover](docs/phase1-handover/README.md), superseding the
earlier "NOT INSTALLED"). Earlier context
also specifies 2.0 TDI quattro 190 HP and physical MMI controls without touchscreen.

Treat supplied identities as known planning inputs. Receiver/graphics/HMI ABI,
real signal/service-loss semantics, safe VC geometry and enhanced cluster behavior
with this adapter beyond the handover's evidence remain unverified. Dashboards are
expected to be limited to the VC map area (handover, INFERRED).
Classic/Extended Map OEM modes are distinct from Compact/Extended dashboards.
No profile/offset is activated from the media-driver label alone. Phase 1 review
and recovery preparation precede hardware dashboard validation; independent host
C01/C05 work remains eligible. No vehicle operations are authorized by these facts.

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
  Report the integration blocker instead of forcing a vehicle implementation;
  under the Phase 2 scope that is a blocked Phase 2, never a completed one.
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

**P2-04 final testing stage (completed, historical):** "Start P2-04, but before make sure you update the documentation, as instructed"
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
