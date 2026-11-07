# P2-FINAL: final Phase 2 host validation

Available host checks completed on 2026-11-07. **Phase 2 remains incomplete:**
no safe Virtual Cockpit surface or target firmware/ABI has been established.
Nothing was deployed, installed, flashed or run on the vehicle; step 9 is blocked.

Claude began this stage under the explicit final-testing request (audit event
`20261107T200748Z-claude-23`) and published b180e36 before inherited regressions.
Codex resumed from that commit, completed those runs, reproduced and fixed a
remaining shutdown defect, and reran the affected host checks. The user then
requested committing/pushing the completed work to the fork's master.

## Results on the resumed executor

Debian 13, GCC 14.2.0, Python 3.12.14, OpenJDK runtime/compiler module 21.0.13.
The javac launcher was absent, but the installed runtime contains `jdk.compiler`.
A temporary launcher invoked the real compiler; no packages were installed.
ASAN_OPTIONS, LSAN_OPTIONS and UBSAN_OPTIONS were unset. Sanitized native runs
used the executor outside the traced sandbox; no sanitizer checks were disabled.

| Check | Result |
| --- | --- |
| `make -C dashboard all test test-sanitize benchmark` | **PASS**, exit 0. Six normal suites (four C, SVG/XML and CLI); four C suites with ASan/UBSan/default leak detection. Saved Compact/Extended previews match generated output. |
| `JAVAC=/tmp/mib2q-p2-final-codex/bin/javac make -C dashboard test-java` | **PASS**, exit 0 after the shutdown fix. Real DashboardConfig/AaRgState/DashboardModule, controlled external-service fixtures, and a Java-produced DBV1 record parsed by C. |
| `JAVAC=/tmp/mib2q-p2-final-codex/bin/javac bash scripts/test_app_lifecycle.sh` | **PASS**, all six scenarios: publication, during-start, replug, failure, bounce, owners. Host Java 21; no claim about stock linkage. |
| `SKIP_JAR_CHECK=1 sh scripts/run_tests.sh` | **PASS**, exit 0, all 39 named checks. Includes AA tee/SIGPIPE, decoder yield, CarPlay hooks, installer/GEM fixtures and lifecycle regressions. Shipping-JAR bytecode check explicitly skipped. Expected inert LD_PRELOAD fixture warnings occurred. |
| `python3 tests/upstream_v141_test.py` | **PASS**, all four methods after correcting the historical preservation assertion's revision scope. Exact upstream release blobs, host decoder policy and real installer/menu upgrade/preference/backups/uninstall fixtures pass. |
| `bash scripts/test_maneuver_native.sh` | **PARTIAL / full script FAILED**, exit 1. Independent lane test passes; Linux compiler rejects the macOS `-framework OpenGL` link. Full macOS parity unavailable here. |
| Direct sanitized `tests/renderer_peer_loss_test.c` | **PASS**, exit 0. Real production disconnect handler over loopback: FIN/RST clear content and frame-ready state. Runs separately because the macOS link stops the native script before this test. |
| `sh tests/renderer_pid_identity_test.sh` | **PASS**, exit 0. Identity/adoption/PID-reuse/fail-open checks. |
| `python3 scripts/check_local_protocols.py` without skip | **FAILED**, exit 1 after 41 matching Java/C constants. Shipping `build/carplay_hook.jar` absent; class-file target validation unavailable. |
| `bash -x scripts/test_java_transports.sh` | **UNAVAILABLE**, attempt exits 2 in stock_env.sh's missing JCL glob under pipefail, before compilation. Stock HMI/JCL assets and stock JDK 8 are absent. Isolated lifecycle harness is covered above. |
| Full shipping Java 1.4/stock linkage, real VehicleDataModule DSI gating and QNX/physical decoder/display tests | **UNAVAILABLE**, proprietary assets/toolchains or vehicle evidence missing. Host fixtures do not prove these behaviors. |

Other stock-dependent Java scripts were not repeated: they share the same absent
HMI/JCL/JDK prerequisites. No stock firmware assets were downloaded or fabricated.

## Defects found and corrected during resume

### Remaining R2 shutdown defect

Before the correction, DashboardModule skipped deleting its published record
whenever another publication was in flight. A blocked second write could leave
the last successful record on disk after stop returned. Existing tests only
blocked a first write, when no prior record existed.

The strengthened test first publishes successfully, then blocks the next open
using a per-generation FIFO. It reproduced an actual failure:
`AssertionError: earlier published record survived stop during blocked write`.

The fix always invalidates the generation and removes the final record on stop.
Bulk file open/write/close remain outside the coordination lock. The final
current-generation check and rename share that lock with stop, preventing a
cancelled writer from committing afterwards. Its temporary file is cleaned by
its owner. The regression verifies immediate clearing, bounded stop while open
is blocked, restart before the old I/O finishes, new-session ownership and no
old writer replacing the restarted output. The updated Java suite passes.

Small filesystem metadata operations (rename/delete) are serialized; this is not
a hard real-time guarantee on an untested QNX filesystem. Failed deletion and
actual target filesystem behavior still need hardware validation. Configuration
changes remain session-bound; deleting the config does not end a running session.

### Historical maintenance test scope

The first maintenance run failed only because it compared the current feature
tree with 6d5e660, forbidding every legitimate later dashboard change. It now
asserts that the actual maintenance commit f5bd2e7 preserved that original tree,
and that the integration remains in HEAD's ancestry. Current dashboard behavior
is covered by the functional suites, not a frozen pre-development tree. The
other release-integrity, hardware-policy and installer checks are unchanged;
all four methods pass. This does not turn a production regression into a skip.

## Review findings and evidence boundaries

| Finding | Current state |
| --- | --- |
| R1 producer clock ordering | Implemented by Claude; current Java/feed fixture passes. Claude's earlier mutation check is historical evidence at b180e36. |
| R2 shutdown/publication | Residual defect reproduced and corrected during resume; strengthened blocked-second-write/restart test passes. Supersedes earlier incomplete R2 acceptance. |
| R3 reader clock ordering | Current C interleaving test and Java/C suite pass. |
| R4 Java lifecycle harness | Repaired by Claude; all six isolated scenarios pass here. Full stock-JAR script remains unavailable. |
| R5 documentation drift | Current guides, pickup instructions and checkpoint reconciled with final results, restored four previews and the updated shutdown contract; original report/audit history retained. |
| Session/sequence liveness | C timeout/backwards-sequence/session-change and manager suppression checks pass. Current owner type does not independently prove an active phone session. |
| Android Auto Kombi-only DSI binding | Implemented in source; real DSI/stock integration remains unverified. |

## Host performance and previews

Fixed benchmark: 20,000 preparations and 2,000 SVG exports per profile.
Preparation 0.90–1.18 microseconds/frame; SVG export 9.02–13.47 microseconds/frame;
manager size 6,400 bytes. Compact uses 30 commands; Extended 42 of 48. These are
measurements on this host only, not MMI CPU/frame-rate predictions.

All four SVG artifacts are present and checked against deterministic CLI output.
They use mock values and host fonts, not a verified VC rectangle or screenshot.

## Reproduce and logs

With a normal host JDK, run `make -C dashboard test-java` and
`bash scripts/test_app_lifecycle.sh` directly. On this executor the temporary
launcher was:

```sh
#!/bin/sh
exec /usr/lib/jvm/java-21-openjdk-amd64/bin/java -m jdk.compiler/com.sun.tools.javac.Main "$@"
```

It lives outside the repository. Native/Java test commands and outcomes are in
the table above; raw local logs are under `/tmp/mib2q-p2-final-codex/`:
`upstream.log`, `maintenance-before.log`, `maintenance-after.log`, `native.log`,
`dashboard.log`, `java-before.log`, `java-after.log`, `lifecycle.log`,
`renderer-peer.log`, `renderer-pid.log`, `protocol-no-skip.log`, `stock-java.log`.
These temporary logs are not a portable deliverable; this report preserves the
results, including failures before correction. No background processes remain.

## Before step 9 or installation

Establish exact MHI2Q train/MU, receiver ABI, hardware/cluster variant and Phase 1
behavior. Prove an available non-critical rectangle, ownership and OEM navigation
coexistence for every intended VC mode; choosing a displayable name is insufficient.
Obtain the appropriate stock assets/toolchain and check full shipping Java/QNX
linkage, read-only DSI validity/units and original AAWireless behavior. No head-unit
commands or installation are authorized by this report. Keep QNX guard/default
OFF and retain the standalone host renderer until those prerequisites are met.
Media omission/capability handling and CarPlay navigation remain explicit gaps
before a real vehicle dashboard can be considered complete. Roadmap Phases 3–5
remain unimplemented; no Android companion or new transport was added.

## Publication

Validated source commit `f1cb7d9322839bdc28557f6346d4201c0a9eb486` was pushed to
the fork master and feature/p2-final-testing; remote SHAs were verified. A final
audit/report documentation checkpoint follows that commit and is discoverable
through Git history. No release or vehicle deployment was performed.
