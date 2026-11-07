# AI continuation guide

Read the canonical [AGENTS.md](../../AGENTS.md) first. This guide is a restart
checklist for any agent or coding tool; it is not a second set of project rules.

For live task state, read [audit/CURRENT.md](audit/CURRENT.md) and recent
[audit/LOG.md](audit/LOG.md) events. The checkpoint below describes P2-01; the active audit records exact
progress and supersedes stale summaries. Follow [the audit lifecycle](audit/README.md), including writing
a RESUME event before making changes.

## Current checkpoint

- The user requested a separate project containing a fork of the original source,
  Markdown architecture/analysis, a phased executable plan and AI instructions.
- The GitHub fork is `alpersid-org/audi-mib2q-dashboards`; use
  `feature/custom-dashboards` for project work.
- The fork contains full upstream history/source. Custom work adds documents,
  agent instructions and the host data/widget module.
- User authorized P2-01 with "Okay, go ahead with the first part." Host module,
  mock source and formatters now exist; do not restart them or request the same
  authorization again. Layout/rendering work is a later step.
- Vehicle firmware/compatibility, original AAWireless behavior, Phase 1 success
  and a safe dashboard surface remain unconfirmed.
- The proposed first implementation is a standalone host renderer with mock
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

For the already authorized [PLAN step 1](PLAN.md#1-establish-the-isolated-module-and-provider),
resume any unfinished acceptance item recorded in audit/CURRENT.md. Read
[module.md](module.md) for the implemented API and test commands. Once complete,
the next eligible task is step 2 (Compact/Extended frame builders and host SVG
adapter), when requested by the user.
Do not start with QNX window creation, context routing, Java lifecycle changes,
GEM buttons, live vehicle feeds or Android communication.

Follow steps 2–5 after their dependencies pass. Vehicle integration remains
blocked under [step 6](PLAN.md#6-vehicle-feasibility-investigation--blocked--future-phase-2-work).
If the user asks to continue without specifying implementation, use the
latest recorded scope/checkpoint and explain the next eligible task; do not silently interpret the old roadmap as permission to code.

## Suggested prompt for another AI tool

> Resume the active task in docs/custom-dashboards/audit/CURRENT.md. Read
> AGENTS.md, the audit protocol/recent log and HANDOFF.md first. Verify the branch,
> commits and working tree against the checkpoint, preserve partial work, and
> append a RESUME event. Continue from the recorded next action within the
> existing user-authorized scope; do not restart completed work or expand scope.
> Read the linked project documents/source as needed. Keep the audit updated at
> milestones and before context exhaustion, with exact test results and the next
> action. Do not contact or deploy to the vehicle.

Append the concrete task, for example "review the architecture document" or,
when implementation is intended, "implement PLAN step 1 on the host."

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
