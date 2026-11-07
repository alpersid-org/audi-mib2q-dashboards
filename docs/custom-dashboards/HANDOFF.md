# AI continuation guide

Read the canonical [AGENTS.md](../../AGENTS.md) first. This guide is a restart
checklist for any agent or coding tool; it is not a second set of project rules.

## Current checkpoint

- The user requested a separate project containing a fork of the original source,
  Markdown architecture/analysis, a phased executable plan and AI instructions.
- The GitHub fork is `alpersid-org/audi-mib2q-dashboards`; use
  `feature/custom-dashboards` for project work.
- The fork contains full upstream history/source. Custom work adds documents
  and agent instruction files; it adds no dashboard implementation.
- Current scope is documentation only. Implementation was explicitly deferred.
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

If later explicitly asked to implement or start the MVP, begin with
[PLAN step 1](PLAN.md#1-establish-the-isolated-module-and-provider): define the
isolated host module and owned snapshot/provider contract, implement a deterministic
mock source and widget formatting, and verify missing/invalid/stale data behavior.
Do not start with QNX window creation, context routing, Java lifecycle changes,
GEM buttons, live vehicle feeds or Android communication.

Follow steps 2–5 after their dependencies pass. Vehicle integration remains
blocked under [step 6](PLAN.md#6-vehicle-feasibility-investigation--blocked--future-phase-2-work).
If the user asks to continue without specifying implementation, apply the
current documentation-only scope and explain the next eligible implementation
task; do not silently interpret the old roadmap as permission to code.

## Suggested prompt for another AI tool

> Read AGENTS.md and docs/custom-dashboards/HANDOFF.md, then inspect PROJECT.md,
> status, requirements, analysis, architecture and PLAN.md. Verify the current
> branch and working tree. Continue only the task I request within the current
> scope. Preserve the safety boundaries, cite source evidence, and distinguish
> confirmed behavior from unverified proposals. Update status with actual results
> and remaining blockers. Do not contact or deploy to the vehicle.

Append the concrete task, for example "review the architecture document" or,
when implementation is intended, "implement PLAN step 1 on the host."

## Completion record for future increments

Maintain these facts in [status.md](status.md), not in an agent's private memory:

- Requested scope and implemented/documented acceptance criteria.
- Changed modules and any architectural decision with its source evidence.
- Exact validation commands and pass/fail/unavailable outcomes.
- Actual artifacts and reproduction commands; no placeholder preview/test claims.
- Open blockers, rollback behavior and next task/dependencies.
- Branch and publication outcome; do not claim a push before verifying it.

Keep AGENTS.md authoritative. Tool-specific instruction files only point to it
so the safety rules and scope do not drift between AI tools.
