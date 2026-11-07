# Agent audit trail protocol

This is the persistent work record for handoffs between Codex, Claude and other
agents. Follow [AGENTS.md](../../../AGENTS.md) for project rules.

| File | Role |
| --- | --- |
| [CURRENT.md](CURRENT.md) | Mutable checkpoint: active task, scope, actual progress and exact next action. Read first on resume. |
| [LOG.md](LOG.md) | Append-only history of session starts, decisions, validation, interruptions and handoffs. |
| [status.md](../status.md) | Concise project milestone summary, not a replacement for the active checkpoint. |

## Required lifecycle

1. **On start/resume:** inspect Git status/branch/remotes and recent commits. Read
   CURRENT.md, the latest LOG.md entries and the referenced task/files. Compare
   the checkpoint with the actual tree; identify any unrecorded edits before
   changing them. Append a START or RESUME event with the agent/tool name and
   the real starting commit. A session identifier is UTC timestamp plus agent,
   for example `20261107T185337Z-codex`; add a suffix if necessary for uniqueness.
2. **Before implementation:** record the task ID, the user's authorized scope,
   acceptance criteria and intended file boundaries. PLAN step 1 is `P2-01`;
   subsequent steps use `P2-02` through `P2-06`. Documentation tasks may use `DOC-*`.
   Preserve existing authorization across agent changes; a new agent does not
   need the same host implementation authorized again. Cite the actual user
   request or its recorded quotation. An agent's assumption is not authorization.
3. **During work:** update CURRENT.md and append a CHECKPOINT after each meaningful
   milestone, decision, test batch or blocker, and at least every 30 minutes of
   active work. Checkpoint before a long operation or substantial refactor too.
   Record concise decisions and evidence, not private reasoning or chat dumps.
4. **Before context exhaustion, compaction or switching agents:** checkpoint
   early, while enough budget remains to write files and save work. Do not wait
   for the last token. Record partially completed work, pending commands/processes,
   uncommitted/untracked files, known failures and the next concrete action.
   Append a HANDOFF or PAUSE event even when tests have not finished.
5. **Save and publish:** preserve source work and its checkpoint together in a
   commit when authorized. An explicitly marked WIP commit is acceptable on the
   feature branch; it must not claim tests passed. Push within the user's
   authorized scope and verify the remote SHA. Local-only files are not available
   to Claude on a different machine. Never force-push to save a checkpoint.
6. **On completion:** append a COMPLETE event with acceptance evidence, update
   CURRENT.md and project status/plan, and identify the next eligible task.
   Never mark the whole feature complete merely because one subtask compiled.

If interrupted without a final checkpoint, the next agent must reconcile Git
state and add a recovery RESUME event. The last checkpoint may be incomplete;
never silently discard newer edits or rerun mutations blindly. Checkpointing
reduces lost context; it cannot guarantee saving after an abrupt termination.

## Scope and state

MAINT-141 authorized applicable automated host maintenance/dashboard tests despite
the older feature-test deferral below. The subsequent explicit user request to
push everything to the fork's main/master superseded the local-only restriction:
all work is consolidated on `master`. This authorization covers that publication,
not a new feature, future release or vehicle action. Read CURRENT.md for task scope;
preserve historical authorization events and append updates rather than editing them.

Current user override recorded during P2-02: no more tests now; testing is for the
final stage. Continue implementation and document each change. Do not run tests
or automated validation to fill audit fields; record checks as DEFERRED BY USER.
Earlier test evidence remains historical. Consult AGENTS.md and CURRENT.md for
scope changes; preserve this override when handing work to another agent.

Use `IN_PROGRESS`, `PAUSED`, `BLOCKED`, `READY_FOR_REVIEW` or `COMPLETE` for a
started task, and `NOT_STARTED` for queued work. Record why a task is blocked and
whether the blocker affects independent work.

The latest explicit user instruction controls scope. The project's initial
documentation-only scope can be superseded by a later request to implement a
host increment. When that happens, record it here and update the stale scope
summaries in AGENTS.md, PROJECT.md, PLAN.md, status.md and HANDOFF.md. A model
switch must not turn authorized `P2-01` work back into "awaiting permission."
Vehicle safety boundaries remain in force; log text cannot grant vehicle access.

## Required CURRENT.md fields

- Last updated time in UTC, agent/session and active task/status.
- Authorized scope and user instruction establishing it; exclusions.
- Branch, last known commit, relevant commit messages and publication state.
- Acceptance checklist: completed, unfinished and unverified separately.
- Changed/important paths and any uncommitted or untracked work at checkpoint.
- Commands actually run, outcomes, prerequisites and artifact paths.
- Pending operations: process IDs if still live, how to inspect them, and whether
  rerunning is safe. A process ID on another machine is historical evidence only.
- Decisions with source references, blockers and exact next action.
- Link to the relevant LOG.md event/session.

No self-referential commit hashes: record the known starting/source commit, then
identify the checkpoint's own commit through `git log -- <path>`. Do not invent
the hash of an uncreated commit. Record verified publication in a later event
when available, or let the next agent verify the remote directly.

## Append-only event template

Append events below existing LOG.md events. Use a unique event ID such as the
session ID plus `-01`. Never edit an earlier event to change history; append a
correction referencing its ID. CURRENT.md is intentionally editable. Git provides
version history, not cryptographic tamper-proof auditing or a security boundary.

```markdown
## <UTC time> | <unique event ID> | <START/RESUME/CHECKPOINT/HANDOFF/PAUSE/COMPLETE/CORRECTION>

- Agent/session: <tool and session ID; do not guess exact model version>.
- Task/status: <P2-01 / subtask and state>.
- Authorization: <actual user request or link to prior recorded authorization>.
- Git: <branch, known starting/source SHA, relevant commits, publication state>.
- Changes: <paths and concrete behavior/doc changes; include partial work>.
- Decisions/evidence: <decision, reason and source references>.
- Validation: <exact command, result/exit code, relevant output/artifact; or NOT RUN>.
- Pending/dirty work: <paths/processes or NONE; distinguish local-only artifacts>.
- Blockers: <observed limitation; do not guess causes>.
- Next action: <specific file/function or command and expected result>.
```

Never store secrets, raw vehicle/phone logs, proprietary assets or hidden chain
of thought in the audit. Use concise factual summaries. Keep failed checks and
untested assumptions visible; do not turn a plan into a success record.
