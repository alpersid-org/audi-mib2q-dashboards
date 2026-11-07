# Current agent checkpoint

Updated: 2026-11-07T21:06:38Z. Agent: Codex.

## Task and authorization

- PUBLISH-REVIEW / IN_PROGRESS. Latest user: "Push and commit". Commit/push
  review documentation to origin/review/claude-phase2; no master merge, fixes,
  tests, release or vehicle actions.
- DOC-REVIEW-HANDOFF / COMPLETE. Latest user requested a Markdown report and
  AI-agent pickup instructions; both are saved and linked from agent entry points.
- Earlier REVIEW-CLAUDE / COMPLETE. User: "Claude has implemented lots of steps. Review
  everything pushed by Claude". No fixes, new feature, publication or vehicle
  actions requested. No tests/new test code under the latest recorded deferral.
- Manual source/diff/document review, not runtime verification.

## Git and files

- Review branch: review/claude-phase2, source HEAD 312215a (origin/master at fetch).
- Range: 574ae26..312215a, 15 Claude commits / 45 changed paths. All fetched origin
  branches are included in that master revision; no separate unreviewed branch tip.
- Local documentation edits only: reviews/claude-312215a.md (report),
  reviews/README.md (pickup guide), AGENTS.md, CLAUDE.md, GEMINI.md, Copilot
  instructions, HANDOFF.md, status.md, PLAN.md, changes.md, audit/CURRENT.md
  and append-only audit/LOG.md. Prior review edits preserved.
- No implementation changes. Documentation commit/push is now authorized and
  pending; no merge. Local master remains 574ae26;
  do not mistake it for the newer reviewed origin/master. No active processes.

## Findings and validation

[Full report](../reviews/claude-312215a.md):

1. R1 P1: Java captures written time before the vehicle accessor timestamps its
   samples; the C reader rejects newer sample times, dropping valid records.
2. R2 P2: stop/delete races an already in-flight write, leaving a fresh record
   after shutdown. Coordinate lifecycle generation and final publication.
3. R3 P2: reader captures wall time before opening the record, so a new publication
   can be incorrectly classified as future and rejected.
4. R4 P2: isolated Java lifecycle build omits new DashboardModule dependency;
   its stubs and owner assertions also need adaptation at the final test stage.
5. R5 P2: current project/provider/architecture/package docs contradict new Java
   shipping changes, implementation state, preview count and active test deferral.

No builds, tests, sanitizer/benchmark/demo runs, preview generation or automated
validation performed. Findings follow from manual source execution-order review;
no frequency or on-car behavior claim. P2-04 results are historical and predate
P2-07/P2-08. Existing dashboard suites are stale; full Java/stock linkage unverified.

## Next action and boundaries

Exact next action: read [the pickup instructions](../reviews/README.md) and
await/follow the user's next task. The report and guide do not start fixes. Use
the ordered queue for a host-only corrective increment when requested. Do not advance into vehicle rendering without resolving these defects
and establishing firmware/ABI/safe-surface evidence. Keep QNX guard/default OFF.
Session identity/sequence validation, media omission and final test repair remain
explicit gaps. No Android app/communication or other roadmap phase implemented.
Publication is now authorized by "Push and commit". Exact current action: commit
and push this documentation to origin/review/claude-phase2, verify remote SHA,
and record completion. Future agents should check out that branch to resume.
Audit: [LOG.md](LOG.md), review events codex-review-claude-01 through -03 and
documentation events codex-review-handoff-01 through -02.
