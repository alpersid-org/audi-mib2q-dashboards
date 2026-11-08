# AI continuation guide

**P2-C05 settings contract (2026-11-08, merged to master):** [settings-contract.md](settings-contract.md) — typed Java `DashboardSettings`, 48 shared conformance vectors run by both C and Java; host tests pass. C05 complete on the host.

**P2-C04 (2026-11-08, host part merged to master):** signal acceptance register [signal-acceptance.md](signal-acceptance.md) and per-signal eligibility (`signal_policy.{h,c}`); production shows ACCEPTED signals only and none is accepted yet; items 2–4 wait for C02 vehicle evidence.

**Latest — C05 review corrections (published, 2026-11-08):** all three findings
corrected in `62b18e1`, fast-forwarded/pushed to fork master; see [resolution](reviews/claude-p2-c05-resolution.md).
Shutdown latches disabled intent; incomplete backends are rejected before taking
a surface. Regression and sanitizer suites pass; Phase 1 planning is reconciled.
User requested "Commit and push to master"; the remote source SHA was verified.

**P2-C05 (2026-11-08, merged to master):** host runtime controller [runtime.md](runtime.md) — states, data-before-ownership, dirty redraw, bounded retry, config/profile reconciler, backend contract; host tests and sanitizers pass. Remaining in C05: shared Java/C settings contract. No native backend; nothing on the vehicle.
Next eligible host work: C05 settings contract or C04 signal policy; C03/C06 stay gated.

**Phase 1 handover (2026-11-08, supersedes "NOT INSTALLED"):** [Phase 1 handover](../phase1-handover/README.md) shows the
upstream mib2q-carplay **1.4.1 release** installed via M.I.B. V3.8.0 and vehicle-
tested: AA cluster map wired and wireless (AAWireless Gen1), Classic and wide view,
OEM functions intact. The unit's `gal` matches upstream profile P5152 (pinned).
Surface constraint (INFERRED, high confidence): dashboards can occupy only the VC
map area; only displayables 98/99 are proven. No post-reboot log set exists yet.

**Target update:** [target-profile.md](target-profile.md) records user-supplied
P5092/MU1329, media driver DEV_MMX2QC_AU_ER_G22_201PROD-1, VC Classic/Extended Map,
Galaxy S23 Ultra and original AAWireless cs307-e2-prod / 3.5.0. MMI Android Auto
works; **Phase 1 is installed** (upstream 1.4.1, vehicle-tested; Phase 1 handover supersedes the earlier "NOT INSTALLED"). Target identity is known as supplied; actual
stock ABI, enhanced cluster behavior, signal validity and safe geometry remain
open. Prepare/validate the original baseline before hardware dashboard validation;
continue independent host work. Older unconfirmed-installation notes are historical.

**Current completion execution plan:** [completion-plan.md](completion-plan.md),
2026-11-08. C01-C09 map every remaining audit point to dependencies and acceptance;
CI/host preparation can proceed while target/surface evidence is collected.
Phase 2 remains incomplete until actual target integration and vehicle acceptance.

**Current audit corrections (2026-11-08):** [completion-audit.md](completion-audit.md)
records installer recovery, widget omission, feed I/O/restart hardening and CI.
Host checks pass; firmware/safe-surface/stock-linkage/vehicle gates remain open.
Phase 2 is not complete. Earlier stage notes below retain their historical scope.

Read the canonical [AGENTS.md](../../AGENTS.md) first. This guide is a restart
checklist for any agent or coding tool; it is not a second set of project rules.

**Current handoff — available host checks complete:** read
[validation-final.md](validation-final.md), [CURRENT.md](audit/CURRENT.md) and
recent audit events first. R4 and stale suites are repaired; R1/R3 and the current
shutdown fix pass host checks. Inherited regressions are complete with documented
platform/asset gaps. The user authorized publication to the fork's master. Do not
restart the old queue or reapply the ended test deferral. Step 9 remains blocked
by missing firmware/ABI and safe-surface evidence; no vehicle execution.

**Earlier review handoff (historical):** read [the report at 312215a](reviews/claude-312215a.md)
and [the agent pickup instructions](reviews/README.md) before choosing work.
R1–R5 are open. This handoff documents an authorized review; it does not start
fixes or step 9. Keep the test deferral and vehicle boundaries below. The report
and instructions are local until Git history confirms later publication.

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

**Review-fix handoff:** see
[reviews/claude-312215a-resolution.md](reviews/claude-312215a-resolution.md);
branch `fix/review-claude-312215a`, merged to master. R4 and stale suites wait for the
final test request.

**Scope handoff (read first):** Phase 2 is "Real Vehicle Data Integration"
([requirements.md](requirements.md#phase-2-scope--real-vehicle-data-integration-authoritative)).
It is **not complete**: P2-01..P2-05 are a mock-only host foundation; P2-07
(omit unverified widgets) is implemented on `feature/p2-07-verified-widgets`
with tests out of date (merged). P2-08 VehicleDataProvider is implemented on
`feature/p2-08-vehicle-provider` ([vehicle-provider.md](vehicle-provider.md)).
Next: step 9 (VC renderer) needs the user's surface decision and head-unit
authorization. Open user decisions: head-unit code authorization and display surface.

**P2-05 handoff (historical):** user request "Start P2-05". Packaging page
[package.md](package.md) on `feature/p2-05-package`; no tests run. After P2-05
the remaining Phase 2 step is 6 (vehicle feasibility), which is blocked; the
user's deferred final test request is pending.

**P2-04 handoff (historical):** the user requested "Start P2-04, but before make sure you update the documentation, as instructed".
P2-04 is the final testing stage; the no-test instruction below has ended. It is
complete on `feature/p2-04-validation` (not merged); results in
[validation.md](validation.md). Next eligible step: P2-05 when requested.

**P2-03 handoff (historical; merged to master on request):** the user requested "Start P2-03 on a dedicated
feature branch". Host configuration and manager are implemented on
`feature/p2-03-config-manager` ([config.md](config.md)); compiled with strict C99,
no tests run (final-stage deferral still active). Not merged to master; merge,
release and vehicle work need separate approval. Next eligible step: P2-04
(automated validation) when the user requests the final testing stage.

**Publication handoff (historical):** the user explicitly authorized pushing all work to the
fork's `master`. Source commit f5bd2e7, including both dashboard steps and MAINT-141,
was fast-forwarded and pushed without conflicts; remote SHA was verified. Read
[the v1.4.1 report](../maintenance/upstream-v1.4.1.md) and current audit before
applying the historical feature checkpoint below. Do not duplicate inherited fixes.
Publication adds no runtime changes or tests and does not start P2-03, authorize
future publication, a release or vehicle actions. Await the next task.

Read [docs/ROADMAP.md](../ROADMAP.md) for all five phases and
[data-providers.md](data-providers.md) for the shared provider architecture.
Current DOC-ROADMAP work is documentation only: no Android communication or live
vehicle provider. Never confuse P2-03 configuration with roadmap Phase 3's
standalone Galaxy S23 Ultra application.

For live task state, read [audit/CURRENT.md](audit/CURRENT.md) and recent
[audit/LOG.md](audit/LOG.md) events. The checkpoint below describes P2-02; the
active audit records exact progress and supersedes stale summaries. Follow
[the audit lifecycle](audit/README.md), including writing a RESUME event before
making changes.

## Current checkpoint

- The user requested a separate project containing a fork of the original source,
  Markdown architecture/analysis, a phased executable plan and AI instructions.
- The GitHub fork is `alpersid-org/audi-mib2q-dashboards`; `master` now contains
  all published project work. Use a dedicated branch for the next feature.
- The fork contains full upstream history/source. Custom work adds documents,
  agent instructions and the host data/widget module.
- P2-01 is complete. User authorized P2-02 with "Okay, push, commit and let's go
  with step 2." Both host layouts, SVG export/CLI and saved previews now exist.
- Historical steering (ended by P2-04): no more tests now; tests are for the final stage. Document
  every change. This overrides earlier run-test instructions across model handoffs.
- Historical target unknowns are superseded by target-profile.md: firmware/adapter
  identities are supplied and Phase 1 is installed (upstream 1.4.1; Phase 1 handover). Actual ABI, enhanced
  behavior and safe geometry still require evidence.
- The implementation is a standalone host renderer with mock
  data, not a new vehicle hook or replacement of navigation graphics.

## Restart checklist

Run these read-only checks from the repository root:

```sh
git status --short --branch
git branch --show-current
git remote -v
git log -5 --oneline
git merge-base --is-ancestor b127988a68c8d690841f4a75d35717ffc01e668e HEAD
git diff --name-status b127988a68c8d690841f4a75d35717ffc01e668e HEAD
```

Then read the documents in AGENTS.md's reading order. If status differs from
this checkpoint, inspect commits and the user's current instructions before
choosing work. Do not discard edits or assume a clean tree.

## Choosing the next task

If asked for more documentation, keep the change within documentation.

P2-01 is complete and P2-02 is implemented. Read [module.md](module.md) and
[rendering.md](rendering.md) for the existing contracts. Follow the active
checkpoint for remaining publication/documentation work. The next implementation
task is step 3 (disabled-by-default configuration and suppression), when requested.
Tests were deferred until the final testing stage; P2-04 has started it.
Do not start with QNX window creation, context routing, Java lifecycle changes,
GEM buttons, live vehicle feeds or Android communication.

Follow later steps according to their dependencies, with testing explicitly
deferred under the current user instruction. Vehicle integration remains
blocked under [step 6](PLAN.md#6-vehicle-feasibility-investigation--blocked--future-phase-2-work).
If the user asks to continue without specifying implementation, use the
latest recorded scope/checkpoint and explain the next eligible task; do not
silently interpret the old roadmap as permission to code.

## Suggested prompt for another AI tool

> Resume the active task in docs/custom-dashboards/audit/CURRENT.md. Read
> AGENTS.md, the audit protocol/recent log and HANDOFF.md first. Verify the branch,
> commits and working tree against the checkpoint, preserve partial work, and
> append a RESUME event. Continue from the recorded next action within the
> existing user-authorized scope; do not restart completed work or expand scope.
> Read docs/ROADMAP.md and the provider design before changing architecture;
> VehicleDataProvider and future AndroidDataProvider must share manager/widgets.
> Read the linked project documents/source as needed. Keep the audit updated at
> milestones and before context exhaustion, with changes, historical test results
> and the next action. Do not run tests or write test code until the user asks
> to implement and run them at the end.
> Do not contact or deploy to the vehicle.

Append the concrete task, for example "review the architecture document" or,
when the next increment is intended, "implement PLAN step 3 on the host without
running tests; document every change."

## Completion record for future increments

Maintain the live facts in [audit/CURRENT.md](audit/CURRENT.md), append history to
[audit/LOG.md](audit/LOG.md), and summarize milestones in [status.md](status.md):

- Requested scope and implemented/documented acceptance criteria.
- Changed modules and any architectural decision with its source evidence.
- Exact validation commands and pass/fail/unavailable outcomes.
- Actual artifacts and reproduction commands; no placeholder preview/test claims.
- Open blockers, rollback behavior and next task/dependencies.
- Branch and publication outcome; do not claim a push before verifying it.

Keep AGENTS.md authoritative. Tool-specific instruction files only point to it
so the safety rules and scope do not drift between AI tools.
