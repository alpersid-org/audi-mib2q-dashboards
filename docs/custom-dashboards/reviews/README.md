# Review handoff for AI agents

Read this file when resuming the review or when the user requests corrections.
It applies to Codex, Claude, Gemini, Copilot and agents using other tools.

## Report and current state

- [Review report: Claude's work through 312215a](claude-312215a.md).
- [Resolution record (2026-11-07)](claude-312215a-resolution.md): R1, R2, R3,
  R5, session invalidation and the AA subscription gap IMPLEMENTED / UNTESTED
  on `fix/review-claude-312215a` (merged to master); R4 OPEN for the final test request.
- [Live checkpoint](../audit/CURRENT.md) and [append-only history](../audit/LOG.md).
- Reviewed range: `574ae26..312215a`, 15 commits and 45 changed paths.
- Five findings remain OPEN. They are source-review findings, not reproduced
  runtime failures. No implementation fixes have been made.
- The report and this handoff were prepared locally on `review/claude-phase2`.
  Read Git status/history to determine whether a later session has published them.
  Another machine needs the documentation commits or a copy of the files first.

The original request authorized this documentation handoff. The subsequent user
request "Push and commit" authorizes publishing it on `review/claude-phase2` to
the fork. It does not start corrective implementation, tests, a master merge or
a release.
Follow a subsequent explicit task when it arrives; do not request approval again
for work the user has already authorized in that session.

## Resume procedure

1. Read [AGENTS.md](../../../AGENTS.md), the live checkpoint, recent audit events,
   and the full report. Follow the audit protocol before editing anything.
2. Inspect the current branch, dirty/untracked files, remotes and recent history.
   Preserve the report, checkpoint and unrelated edits. In the original review
   workspace, local `master` was older than `origin/master`; inspect the actual
   refs rather than assuming a branch name identifies the reviewed revision.
3. Compare the current source with `312215a`. If it has advanced, inspect changes
   to each cited function before deciding whether a finding remains applicable.
   Do not blindly apply a patch based on old line numbers. If Git history or
   source files are missing, record the limitation instead of guessing.
4. Append a START/RESUME event with your agent/session, real starting SHA, task,
   actual user authorization, intended paths and validation policy. Update
   CURRENT.md. If only a review/documentation task is authorized, retain that
   scope; otherwise use a dedicated corrective branch and preserve existing work.
5. For an authorized corrective task, follow the order below. Keep one finding
   per reviewable increment where practical; avoid unrelated refactors.
6. Update CURRENT.md, LOG.md and the change ledger after each meaningful
   increment and before handing off. Record partial files, pending processes,
   blockers and the exact next action before context exhaustion.

These are read-only orientation commands, not tests:

```sh
git status --short --branch
git branch -avv
git remote -v
git log -8 --oneline
git diff --name-status 312215a HEAD
```

## Corrective work queue (requires an implementation request)

| Order | Finding | Intended change | Acceptance evidence to record |
| --- | --- | --- | --- |
| 1 | R1 / P1: producer timestamps | Collect vehicle/navigation snapshots before capturing the record's write time. Preserve rejection of genuinely future/corrupt samples. | Source ordering makes every contributing sample no newer than the write time. At final testing: clock-tick boundary and Java-to-C record fixtures. |
| 2 | R2 / P2: late publication after stop | Tie the writer to a lifecycle generation and serialize publication/removal so an old iteration cannot recreate or replace a stopped/new session's record. Keep lifecycle waits bounded. | State transitions prevent publication after shutdown completes, including immediate restart. At final testing: controlled stop during collection/write/rename, concurrent restart and failure recovery. |
| 3 | R3 / P2: reader timestamps | Obtain the validation wall time after acquiring the complete record; retain conservative age mapping to the manager clock. | A publication during file acquisition is not classified as future solely because the reader sampled its clock earlier. At final testing: deterministic interleaving and age/future boundaries. |
| 4 | R5 / P2: documentation drift | Reconcile current project, architecture, provider, package and rollback guides with the changed shipping Java, implemented manager/provider, two remaining previews and active no-test policy. | Current-state claims match source; older outcomes are revision-pinned history. Configuration deletion is documented as session-bound, not immediate deactivation. |
| Final test request | R4 / P2: Java lifecycle harness, plus existing test debt | Repair isolated dependencies/stubs and owner/gating expectations. Update suites referencing removed trip/consumption widgets; add feed/Java-C/lifecycle coverage and regenerate Extended previews. | Record actual commands/results; distinguish missing prerequisites, failures and passes. Historical P2-04 results do not validate newer code. |

The report contains source references, failure scenarios and suggested remedies.
Do not weaken validation or remove checks merely to silence a failure. A finding
may be challenged with concrete source evidence; record why rather than silently
discarding it. Reconcile session identity/sequence invalidation before a live
consumer is connected; owner type and fresh timestamps alone do not identify a
specific phone session.

## Validation policy and completion evidence

**Current:** the user started the final Phase 2 host testing stage (P2-FINAL), which
authorizes tests and test code on the host, including the R4 repair. The deferral
text below is historical.

The latest recorded user instruction is: "do not run any test, there will be a
ask at the end to implement and run the tests". Until explicitly superseded,
do not write test code or execute tests, sanitizers, benchmarks, demos, preview
generation or automated validation. Do not build as a substitute for testing.
Use manual source/document review and clearly record DEFERRED BY USER.

Track each finding separately as OPEN, IMPLEMENTED / UNTESTED, VERIFIED BY HOST
TESTS, or DISPUTED WITH EVIDENCE. Record implementation commit/path, manual
evidence, deferred checks and remaining limitations. When tests are authorized,
use the report's focused cases and relevant inherited regressions; inspect each
script before execution. Missing stock HMI assets/JDK/QNX hardware are explicit
prerequisite gaps, never passes. Preserve default sanitizer settings and report
environment adjustments honestly.

Keep the original review pinned to `312215a`. Add a dated resolution record next
to it and link that record from this index after corrective work; do not rewrite
the original report to imply the defects never existed. LOG.md stays append-only.
Commit/push only within the latest user-authorized scope and verify remote SHAs
before reporting publication. Never force-push or use the upstream remote.

## Safety and roadmap boundaries

This remains roadmap Phase 2. Keep the generic Data Provider Interface, shared
manager/widgets, QNX compilation guard and disabled default. Do not implement
an Android companion app, new Android-to-MMI communication or DPF monitoring.
Preserve upstream AA/CarPlay behavior and OEM navigation/instrumentation.

Do not begin step 9 solely because a surface was selected by name. Firmware/ABI
and a proven non-critical rectangle/ownership policy remain prerequisites.
Do not execute commands on, install, flash or deploy to the head unit. Host
changes and tests cannot establish compatibility or safety on this Audi.

## Prompt to give the next agent

> Read AGENTS.md, docs/custom-dashboards/audit/CURRENT.md, recent audit events,
> docs/custom-dashboards/reviews/README.md and the linked review report first.
> Reconcile the reviewed revision 312215a with the current branch and source.
> Preserve all existing edits and append an audit RESUME event. Follow my current
> task; the handoff itself does not authorize implementation or publication.
> Keep tests and new test code deferred until I explicitly request them. For an
> authorized corrective task, address R1, R2, R3 and R5 in that order, documenting
> each change; retain R4 and the stale suites for the final test stage. Do not
> start the vehicle renderer, deploy, or implement roadmap Phases 3–5.
