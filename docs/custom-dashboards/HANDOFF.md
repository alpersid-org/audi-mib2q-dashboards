# AI continuation guide

Read the canonical [AGENTS.md](../../AGENTS.md) first. This guide is a restart
checklist for any agent or coding tool; it is not a second set of project rules.

**P2-04 handoff (current):** the user requested "Start P2-04, but before make sure you update the documentation, as instructed".
P2-04 is the final testing stage; the no-test instruction below has ended. Work is
on `feature/p2-04-validation`; read the audit checkpoint for progress.

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
- Vehicle firmware/compatibility, original AAWireless behavior, Phase 1 success
  and a safe dashboard surface remain unconfirmed.
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
> and the next action. P2-04 (final testing stage) authorizes host tests only.
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
