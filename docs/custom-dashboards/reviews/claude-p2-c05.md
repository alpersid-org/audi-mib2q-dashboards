# Review of Claude's P2-C05 push

**Follow-up:** user requested "Fix all findings"; all three are corrected in `62b18e1`, published to fork master
with evidence in the [resolution record](claude-p2-c05-resolution.md). The review
below remains pinned to the pushed revision and preserves its original findings.

Reviewed by Codex, 2026-11-08. User: "Check the last push from Claude.
FYI - Phase 1 was done". Fork master reviewed at
`28dab92461c69056ed162e4b2d4563b3b75d1689`: implementation `f9c1495` and
documentation `28dab92`, compared with parent `1a2d1cd`. Local review branch:
`review/claude-p2-c05`. No production source changes, commits or pushes.

**Assessment: changes needed in the controller lifecycle.** Two P2 defects
reproduce against the pushed code although its existing C/sanitizer tests pass.
There is also a stale Phase 1 statement in the target-profile documentation.

Phase 1 is complete: the supplied handover records upstream mib2q-carplay 1.4.1
installed and vehicle-tested, with wired/wireless AA cluster maps and OEM checks.
This review accepts that baseline. P2-C05 is host work; the shared Java/C settings
contract and native backend remain explicitly unfinished, not new review findings.

## R1 — P2: a tick after shutdown silently restarts output

Location: [runtime.c:131–135](../../../dashboard/runtime.c#L131), with the enable
gate at [runtime.c:93](../../../dashboard/runtime.c#L93). Introduced by `f9c1495`.
Status: **OPEN / REPRODUCED ON HOST**.

`db_runtime_shutdown` calls `leave(..., DB_RT_DISABLED)` but leaves both runtime
and manager configurations enabled. The next periodic tick passes the enable
gate, reads the still-fresh record, acquires the surface and draws again without
any enable/restart request. This violates the documented disabled-after-shutdown
behavior and the C05 no-late-output lifecycle expectation when a scheduled tick
arrives after shutdown. This is single-threaded and does not require a race.

Reproduction, using the pushed test's DBV1 publisher and fake backend:

1. Initialize with a full backend, set WIDE profile, apply enabled Compact config,
   publish a fresh valid record, and tick at 100000: ACTIVE, acquire/draw once.
2. Call shutdown: DISABLED, cleared and released as expected.
3. Tick at 100250 without changing configuration: **ACTIVE, owned=1, visible=1,
   acquires=2, draws=2**. Expected DISABLED and no further backend calls.

Make shutdown latch disabled intent (or an explicit stopped state) until a
documented restart operation. Test ticks after shutdown and the intended restart
path. The existing test only calls shutdown twice and inspects immediate cleanup.

## R2 — P2: an incomplete backend can acquire a surface it cannot release

Location: [runtime.c:105–113](../../../dashboard/runtime.c#L105), cleanup at
[runtime.c:8–10](../../../dashboard/runtime.c#L8). Introduced by `f9c1495`.
Status: **OPEN / REPRODUCED ON HOST**.

The runtime accepts a backend with working acquire/draw/clear and NULL release.
It reaches ACTIVE normally. On disable, `leave` skips the missing release
callback and resets `rt->owned` to zero, even though the backend still owns the
surface. OEM restoration cannot be guaranteed, and later reacquisition can
attempt to take an already-owned surface. The API does not document a non-NULL
precondition and explicitly handles missing callbacks, so this must fail closed
before acquiring, rather than silently treating cleanup as optional.

Reproduction: initialize the pushed fake backend with only `release = NULL`,
enable and publish/tick to ACTIVE, then apply disabled configuration. Observed:
**state=disabled, backend_owned=1, runtime_owned=0, releases=0**. Expected rejection
of that backend before the first acquisition. Validate all required callbacks
(including clear and release) before granting ownership, and cover partial
backend combinations. The all-NULL backend test does not cover this case.

## R3 — P3: the target profile still says Phase 1 was not performed

Location: [target-profile.md:43–45](../target-profile.md#L43), with outdated
execution steps at [target-profile.md:82–96](../target-profile.md#L82).
Status: **OPEN / CONFIRMED BY SOURCE**. This is a handover reconciliation issue
in the current documentation, outside the two-commit controller diff.

The banner and table correctly say installed and vehicle-tested, but the body
still says "Phase 1 is explicitly not performed" and that it must not be
described as complete. It then instructs agents to establish Phase 1 separately.
Replace those current-state instructions with the completed upstream-release
baseline and the genuinely remaining post-reboot/recovery evidence. Do not
reopen Phase 1 installation or equate the fork's dashboard build with the
installed upstream release. The handover's original documents remain unchanged.

## Validation

Inspected the dashboard Makefile, controller suite and Java test script before
running host checks. No vehicle commands, installation or deployment occurred.

| Command/check | Result |
| --- | --- |
| `make -C dashboard all test test-sanitize test-java` | C build and all normal C/XML/CLI/runtime tests **PASSED**; combined command exited 2 at LeakSanitizer's ptrace restriction before reaching Java |
| `make -C dashboard test-sanitize` outside traced sandbox | **PASSED**, all five ASan/UBSan suites, with default leak checks retained |
| `make -C dashboard test-java` | **UNAVAILABLE**: `javac: command not found`; make exit 2, script error 127; Claude's Java result was not independently verified |
| `cc -O2 -g -std=c99 -Wall -Wextra -Werror -pedantic /tmp/claude-c05-repro.c build/dashboard/libdashboard.a -o /tmp/claude-c05-repro` | **PASSED** |
| `/tmp/claude-c05-repro shutdown` | Exit 1, **R1 reproduced** with ACTIVE output after shutdown |
| `/tmp/claude-c05-repro missing-release` | Exit 1, **R2 reproduced** with backend ownership retained after disable |
| `git diff 1a2d1cd..28dab92 --check` | **PASSED** |
| GitHub combined commit status for `28dab92` | Connector returned `statuses: []`; no remote check success inferred |

The temporary reproduction source includes the pushed `dashboard_runtime_test.c`
with its main renamed, reuses its real DBV1 fixture and fake callbacks, and adds
the public-API sequences above. It links the unmodified pushed library. Source
and binary are local `/tmp` review artifacts; the build log is
`/tmp/claude-c05-host-checks.log`. No persistent test or implementation was added.

Host findings establish API/ownership defects, not observed vehicle failures.
QNX/default-OFF guards remain intact. Next corrective work: fix R1/R2 with
regressions and reconcile R3 before treating C05 lifecycle acceptance as met.
