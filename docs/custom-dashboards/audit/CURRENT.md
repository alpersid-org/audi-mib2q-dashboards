# Current agent checkpoint

Updated: 2026-11-07T19:52:53Z. Agent: Codex.
Session: `20261107T194002Z-codex`.

## Active task and authorization

- Task/state: `MAINT-141` / `READY_FOR_REVIEW`, local only.
- User requests official upstream v1.4.1 (b127988) maintenance integration and
  applicable automated tests, preserving Phase 2 dashboards and the full roadmap.
- Host maintenance/dashboard testing explicitly authorized; earlier test deferral
  is superseded for this task. No vehicle actions, automatic push, main/master
  merge or release. Keep changes local and prepare for review.

## Git/source evidence

- Started clean at `6d5e66021b5f511e499da35557af67bdf213b75d`.
- Dedicated local branch: `maintenance/upstream-v1.4.1`.
- Exact baseline already v1.4.1: annotated tag peels to
  `b127988a68c8d690841f4a75d35717ffc01e668e`, ancestor of existing branch.
- Reviewed complete 20-file v1.4..v1.4.1 diff and official remote release/tag/
  changelog. Runtime/deploy/installer files match target unchanged.
- Commits e6ac2b9, d8bbfa0, b127988 already included; no cherry-pick/merge needed.
- Preserve all dashboard sources/interfaces/previews and docs/ROADMAP.md unchanged.

## Acceptance

- [x] Inspect branch/relationships, complete release diff and existing integration.
- [x] Confirm upstream tag/release and exact runtime source preservation.
- [x] Run applicable upstream tests including AA tee/SIGPIPE, yield, lifecycle,
  GEM, installer and dashboard tests; record missing proprietary/platform checks.
- [x] Add targeted upgrade/preference coverage if absent.
- [x] Review/report source mapping, conflicts, risks and future vehicle test gates.
- [x] Prepared local review increment; source commit discoverable with
  `git log -1 -- tests/upstream_v141_test.py`. No push/merge/release authorized.

## Files and next action

Implemented changes: tests/upstream_v141_test.py, docs/maintenance/upstream-v1.4.1.md
and agent/project/status/plan/handoff/change-ledger/audit docs. No runtime rewrite.
The complete file map and source/test evidence are in the maintenance report.

Tests: upstream 39 named host checks ran to exit 0 with shipping-JAR check explicitly
skipped; dashboard C/XML/artifact and ASan/UBSan passed; four new maintenance tests
passed; renderer PID identity and sanitized FIN/RST recovery passed. Tests ran
outside traced sandbox as needed with sanitizer defaults intact. Initial new
fixture generation failed for a missing temp dispatcher resource, corrected in
fixture only; final suite passed. No-skip shipping check cannot pass without JAR.
Native lane test passed, macOS -framework link failed on Linux; Java setup failed
on missing JCL/HMI/JDK. No local Docker images/QNX toolchain or FFmpeg dev packages.
No vehicle compatibility/decoder/visual acceptance claim follows from host checks.

Final documentation check: 21 Markdown files, 255 relative links and anchors/fences
passed; git diff --check and release/custom preservation checks passed. No test
processes remain. No packages installed or release artifacts created.

Exact next action: review the local branch diff/report and await the user's
approval before any push, main/master merge or release. Do not reapply already
inherited upstream commits, discard Phase 2 work or begin another roadmap phase.
Checkpoint commit: discover through Git history (no self-referential hash).
Audit: [LOG.md](LOG.md), session `20261107T194002Z-codex`.
