# Current agent checkpoint

Updated: 2026-11-08T19:45:33Z. Agent: Claude Code. Task: P2-C01 CI diagnosis / BLOCKED (owner action)
on `feature/p2-c01-ci` (from master ab97450). Not merged.

- GitHub API: workflow "Tests" active, 0 runs ever; workflow_dispatch from this
  session -> 403 (agent token lacks actions write). Likely cause: Actions disabled
  on the fork (or org policy). Owner must enable Actions; see ci-status.md.
- Local workflow-equivalent run (fresh clone, clang 18, ksh/mksh, SKIP_JAR_CHECK=1):
  run_tests 38/39 PASS (hw301 needs IPv6, absent here); dashboard all/test/
  sanitize/java, lifecycle, installer recovery (10), upstream v1.4.1 (4) PASS.
- Container changes (local only): installed ksh, mksh, zlib1g-dev, libclang-rt-18-dev.

Next: user enables Actions and starts a run; then inspect run/jobs/logs and fix
real failures. Audit: [LOG.md](LOG.md).
