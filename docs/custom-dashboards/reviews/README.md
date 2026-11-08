# Review handoff for AI agents

Read this file when resuming the review or when the user requests corrections.
It applies to Codex, Claude, Gemini, Copilot and agents using other tools.

## Report and current state

- Latest review: [Claude P2-C05 at 28dab92](claude-p2-c05.md), 2026-11-08.
  [Resolution](claude-p2-c05-resolution.md): all three findings corrected in
  `62b18e1`, published to fork master. Shutdown stays disabled until explicit enable;
  incomplete backends are rejected before ownership. Regression and sanitizer
  suites pass; Phase 1 instructions reconciled. Remote source SHA verified.
  Phase 1 is installed and vehicle-tested. The R1/R2 identifiers in this report
  are separate from the historical 312215a findings below.
- [Original review at 312215a](claude-312215a.md) remains revision-pinned.
- [Resolution history](claude-312215a-resolution.md) and
  [final host validation](../validation-final.md) describe current evidence.
- R1/R3, the R4 harness and current dashboard/feed tests pass on the host. A
  remaining R2 shutdown defect was reproduced and corrected during resume; the
  strengthened blocked-second-write/restart test passes. R5 guides are updated.
- Full stock Java/DSI/QNX linkage and vehicle behavior remain unverified. Host
  results do not establish safe VC composition; step 9 stays blocked.
- Use [CURRENT.md](../audit/CURRENT.md), Git and recent [events](../audit/LOG.md)
  for branch/publication status. The user requested finishing remaining host
  work and committing/pushing to the fork's master; no release/vehicle actions.
- On resume, reconcile newer source with the reviewed/fixed revisions. Never
  replay the old OPEN queue or reinstate an obsolete no-test policy blindly.

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

## Original corrective queue (historical; use final results above)

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

The explicit P2-FINAL testing request superseded the earlier no-test policy.
Host tests, test code, harness repair and checks for affected changes are
permitted. Available host validation is complete; repeat only for new changes,
failures or unresolved concerns. Record missing prerequisites as UNAVAILABLE,
not passes. Firmware/surface evidence and vehicle actions remain separate gates.

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
> task and current final validation report. The old test deferral is superseded;
> host tests are authorized, and R4/stale suites have been repaired. Preserve
> prior source and test evidence; do not restart resolved work from the historical
> queue. Record each new change and its checks. Do not start the vehicle renderer
> without proven firmware/surface prerequisites, deploy, or implement Phases 3–5.
