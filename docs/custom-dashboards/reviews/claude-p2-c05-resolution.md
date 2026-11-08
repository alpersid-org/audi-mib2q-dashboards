# P2-C05 review resolution

User: "Fix all findings", 2026-11-08. This resolves the three findings in the
[review pinned to 28dab92](claude-p2-c05.md). Branch:
`fix/claude-p2-c05-review`, based on `28dab92461c69056ed162e4b2d4563b3b75d1689`.
Committed as `62b18e13d758a886753afd86f1ab0c0b001da328`, fast-forwarded into master
and pushed to the fork on the user's "Commit and push to master". `git ls-remote`
verified that exact remote SHA. Publication checkpoint follows in a docs commit.

| Finding | Disposition | Correction and evidence |
| --- | --- | --- |
| R1 / P2: shutdown restarts output | **VERIFIED BY HOST TESTS** | `db_runtime_shutdown` disables runtime and manager configs and cancels retry state. Ticks remain disabled until explicit enabled config is applied; layout is retained. Regression exercises fresh publications after shutdown without reads/backend calls, explicit restart, and shutdown/restart during draw-failure backoff. |
| R2 / P2: incomplete backend leaks ownership | **VERIFIED BY HOST TESTS** | The acquisition gate requires acquire/draw/clear/release before invoking acquire. All 15 incomplete callback combinations reach recovering without any output callback calls; checks include retries, disable and shutdown. Existing full-backend tests still verify draw-failure cleanup and every granted surface released. |
| R3 / P3: stale Phase 1 instructions | **CORRECTED / CHECKED** | Target-profile body and execution order accept installed upstream 1.4.1 as the completed vehicle-tested Phase 1 baseline. Completion plan and roadmap no longer request Phase 1 installation; historical analysis unknowns are explicitly dated and linked to current evidence. Stock/geometry/signal/post-reboot/recovery gaps remain explicit. Original user handover documents unchanged. |

## Validation

- Added regression tests before the runtime fix. Focused runtime suite failed
  on the original shutdown behavior (exit 134). After fixing R1 alone, it failed
  on incomplete-backend ownership (exit 134). Both regressions then passed with
  R2 fixed. Core dumps were disabled for these intentional failing runs.
- `make -C dashboard all test`: **PASSED**, normal C/widget/layout/manager/feed/
  runtime suites, SVG/XML and CLI checks.
- `make -C dashboard test-sanitize`: **PASSED**, all five suites under ASan/UBSan,
  outside the traced sandbox so LeakSanitizer can run; no checks disabled.
- Java: **UNAVAILABLE** on this host (`javac` absent during review). Java source,
  configuration formats and record protocol are unchanged; no new Java pass claimed.
- Documentation links/fences: **PASSED**, 14 changed/new Markdown files and
  418 local links. Current profile/plan/roadmap have no obsolete Phase 1
  installation instructions. `git diff --check`: **PASSED**.
- Logs: `/tmp/claude-c05-fix-tests.log`, `/tmp/claude-c05-fix-sanitize.log` (local).

The runtime backend remains a host contract. Native/QNX integration and shared
typed Java/C settings are still separate unfinished C05/C06 work. No vehicle
action, installation or release occurred. Remote publication is recorded above.
