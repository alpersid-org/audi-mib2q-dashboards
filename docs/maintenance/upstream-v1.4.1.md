# Upstream v1.4.1 maintenance review

Review date: 2026-11-07. Working branch: `maintenance/upstream-v1.4.1`.
Starting project commit: `6d5e66021b5f511e499da35557af67bdf213b75d` on
`feature/custom-dashboards`. The working tree was clean.

## Publication update (2026-11-07)

After the local review, the user requested "Push everything to main/master on my
forked project". Master was fast-forwarded without conflicts from b127988 to
`f5bd2e72290a1509d63f831e7dce4dc5282afaa9` and pushed to origin; the remote SHA was
verified. All existing dashboard and roadmap work is included. A subsequent
publication documentation checkpoint is discoverable in Git history. The report
below records the original maintenance increment, which changed no runtime code.
This supersedes its local-only restriction, not the vehicle/release restrictions.

## Outcome

**The complete target release was already integrated before this task.** The
project was originally forked from
`b127988a68c8d690841f4a75d35717ffc01e668e`, exactly the requested v1.4.1 commit.
All 20 files changed since v1.4 match the target release byte-for-byte in the
current working tree. The target is an ancestor of our custom work; no missing
fix, duplicate cherry-pick, merge or production rewrite is needed.

This local maintenance increment adds targeted regression coverage, records the
complete source audit and updates agent/status instructions. It preserves all
dashboard code, interfaces, previews, the five-phase roadmap and provider design.
No upstream runtime, installer, GEM dispatcher or release files were modified.
At the original maintenance-review checkpoint, no push, main/master merge,
release packaging/publication or vehicle action was performed. The later approved
master publication is recorded above. This is maintenance, not a new roadmap phase.

## Upstream identity and full diff

Official resources reviewed:

- [Release page](https://github.com/kamgurgul/mib2q-carplay/releases/tag/v1.4.1)
  and its release-body metadata through the read-only GitHub API.
- [Requested changelog](https://github.com/kamgurgul/mib2q-carplay/blob/master/docs/release/CHANGELOG-1.4.1.md),
  also compared with the local pinned release documentation.
- Remote annotated tag object `b57f8fdfd4dc260019f7bec79ba3e6daddcac391`,
  confirmed to point to commit `b127988a68c8d690841f4a75d35717ffc01e668e`.
- v1.4 peels to `5c460600c9205cdafa2c4fecd7e2210b8606aedb`.

The complete diff `git diff v1.4..v1.4.1` was reviewed, including release docs:
20 files, 649 insertions and 50 deletions. All intervening commits are already
ancestors of our starting project commit:

| Existing upstream commit | Contribution |
| --- | --- |
| `e6ac2b9a4522a97005205b60786d760b8d651b0a` | AA main-screen yield, tee SIGPIPE protection, immediate HW/opt-in delayed policy, menu/docs and host regressions. |
| `d8bbfa00b80af1c39ee6be83f13e01d95104f85c` | Apple-specific SIGPIPE handling in the tee regression test. |
| `b127988a68c8d690841f4a75d35717ffc01e668e` | Release packaging default/version updated to 1.4.1. |

**New upstream commits incorporated by this task: none; all three were already
present. Conflicts: none; no merge/cherry-pick was attempted.**

Reproduction of source evidence (read-only):

```sh
git rev-parse 'v1.4^{}' 'v1.4.1^{}'
git merge-base --is-ancestor b127988a68c8d690841f4a75d35717ffc01e668e HEAD
git log --reverse --format='%H %s' v1.4..v1.4.1
git diff --stat v1.4..v1.4.1
git diff v1.4..v1.4.1
```

The new maintenance test verifies the peeled identities, ancestry and actual
working-tree blob hashes for every file in the 20-file release delta. This is a
pinned integration audit; a later intentional fork modification to those files
requires reviewing that equality expectation rather than deleting a failure.

## Complete 20-file release mapping (already present, unchanged)

| Source | Verified contribution / limit |
| --- | --- |
| [aa_hook/aa_tee.c](../../aa_hook/aa_tee.c) | Writer thread blocks SIGPIPE locally; a disconnected/reset renderer drops only its client. Receiver-wide signal dispositions are untouched. |
| [aa_hook/aa_yield.c](../../aa_hook/aa_yield.c) | Monotonic yield stamp written before checking active decoder; waits at most 1500ms for release. Missing/stale renderer returns promptly; incomplete active files treated conservatively. |
| [aa_hook/aa_hook.c](../../aa_hook/aa_hook.c) | Main-sink setup/codec/start/stop and receiver shutdown announce yield, gated by hook enablement/main-sink identity. Original callees/return values forwarded; no video-focus yield added. |
| [aa_hook/aa_hook.h](../../aa_hook/aa_hook.h) | Shared yield/active paths, wait bound and yield API. |
| [aa_hook/aa_hook.exports.map](../../aa_hook/aa_hook.exports.map) | Exports lifecycle interposers required by upstream implementation. No additional hooks introduced by this maintenance task. |
| [altscreen_render/main.c](../../altscreen_render/main.c) | AA uses the same optional delayed-start marker as CarPlay, removing the unconditional 120s wait. Existing 3s quiet hold, 40ms yield polling, active-before-yield claim, watchdog, cooldown and software fallback remain intact. |
| [deploy/gem_menu/run.sh](../../deploy/gem_menu/run.sh) | Same delayed-start marker, absent means OFF; messages now cover CarPlay and AA. Existing enabled preferences retain their meaning and now also apply to AA. |
| [scripts/build_aa_hook.sh](../../scripts/build_aa_hook.sh) | Links aa_yield.c into the AA hook. |
| [scripts/build_gem_menu.sh](../../scripts/build_gem_menu.sh) | Label specifies CarPlay + Android Auto and default OFF. |
| [scripts/build_release.sh](../../scripts/build_release.sh) | Existing default version 1.4.1; no release generated here. |
| [scripts/run_tests.sh](../../scripts/run_tests.sh) | Adds yield regression and CarPlay/AA/renderer marker agreement checks. |
| [tests/aa_hook_gate_test.c](../../tests/aa_hook_gate_test.c) | Enabled/main-sink gating, teardown, original forwarding/return-value checks. |
| [tests/aa_tee_test.c](../../tests/aa_tee_test.c) | Default SIGPIPE disposition, signal to writer thread, RST client loss and continued client service; includes Apple-host handling. |
| [tests/aa_yield_test.c](../../tests/aa_yield_test.c) | Stamp ordering/format, absent/dead/releasing/nonreleasing renderer, bounded wait and half-written file behavior. |
| [README.md](../../README.md) | Version and dual-phone delayed-start description. |
| [docs/altscreen/hw-decoder.md](../altscreen/hw-decoder.md) | AA yield, opt-in boot settle, existing recovery/fallback. |
| [docs/android-auto/overview.md](../android-auto/overview.md) | SIGPIPE/yield behavior and shared delayed marker. |
| [docs/deploy/altscreen-mhi2q.md](../deploy/altscreen-mhi2q.md) | Marker present/absent policy for both phone types. |
| [docs/release/CHANGELOG-1.4.1.md](../release/CHANGELOG-1.4.1.md) | Complete bug-fix/change documentation, including pending car validation. |
| [docs/release/RELEASE_NOTES-1.4.1.md](../release/RELEASE_NOTES-1.4.1.md) | Update/GEM-label guidance, preference preservation and explicit car-test caveat. |

There is **no installer/mod code change in the v1.4 → v1.4.1 diff**. Installer,
backup and uninstall behavior are inherited intact. The release changes the AA
hook, renderer, menu dispatcher/label, build wiring and documentation. An updated
menu needs the menu setup path as well as program file updates; the GEM program
update button alone does not update the menu. This report does not execute either
path on a real unit.

## Test results

All commands run on the host. Native socket/sanitizer suites used the execution
environment outside its traced sandbox so default leak checks could operate;
no sanitizer was disabled. No package/toolchain was installed.

| Command | Result and actual coverage |
| --- | --- |
| `SKIP_JAR_CHECK=1 sh scripts/run_tests.sh` | PASS, exit 0. All 39 named checks reached, including AA tee/SIGPIPE, yield, gate/profile, startup lifecycle, marker agreement, map/settings, CarPlay bus/cover/route, supervisor/recovery, GEM and installer fixtures. Shipping-JAR class version check explicitly skipped; no Java pass implied. |
| `make -C dashboard all test` | PASS. Existing widget/data and layout/export tests; independent XML/CLI checks also confirm all four saved previews match deterministic output. Source previews not regenerated. |
| `make -C dashboard test-sanitize` | PASS, both C suites under ASan/UBSan with default leak detection. |
| `python3 tests/upstream_v141_test.py` | PASS, four test methods: exact upstream identity/20-file hashes; dashboard/roadmap/provider preservation; production renderer policy functions/expressions on host; v1.4→1.4.1 program + GEM upgrades with preference/backup preservation. |
| `sh tests/renderer_pid_identity_test.sh` | PASS, stale adoption, PID reuse, registry/probing and signaling policy. |
| Host compile/run of `tests/renderer_peer_loss_test.c` with ASan/UBSan | PASS, real loopback FIN/RST clears stale readiness/content. No actual QNX compositor/decoder involved. |
| `python3 scripts/check_local_protocols.py` without skip | 41 source constants match, then exit 1 because build/carplay_hook.jar is missing. This invocation is not an overall pass. |
| `bash scripts/test_maneuver_native.sh` | Lane receiver test passed; script then exit 1 on Linux because compiler does not support macOS `-framework OpenGL`. Full GL/scene runtime suite unavailable. |
| `bash -x scripts/test_route_info.sh` | Exit 2 in stock_env.sh: missing JCL glob under pipefail, before Java test execution. HMI jar/JCL and stock JDK absent. No Java behavior verified. |

The new upgrade fixture uses actual v1.4 and current installer/menu-generation
sources, staged below `/tmp`, with all `/mnt` paths redirected and mount/sync
operations disabled. Payloads are inert text; they prove copy/version/backup
semantics, not executable firmware behavior. It checks delayed-start absent/OFF
and present/ON on `/bin/dash` and `/bin/sh`, upgrades, repeats program/menu install,
preserves bytes of delay/engine/media/HW preferences, verifies updated labels,
retains original stock backups and restores stock JSON on uninstall. QNX pdksh
and FAT media remain outside this host evidence.

The renderer policy fixture extracts production functions/conditions verbatim;
only paths and monotonic clock are fixtures. It checks default immediate/opt-in
delay, separate AA boot-only versus CarPlay persistent fallback, yield claim/hold,
cooldown exhaustion and recovery-record clearing. It does **not** start OMX,
emulate Venus, run the decoder/compositor loop or prove on-device synchronization.

Peer-loss reproduction command:

```sh
cc -std=gnu99 -O1 -g -Wall -Wextra -Icommon \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tests/renderer_peer_loss_test.c -lpthread -o build/dashboard/maintenance_peer_loss
build/dashboard/maintenance_peer_loss
```

Expected dummy LD_PRELOAD warnings in upstream fixture tests do not represent
loads of actual hooks. Local raw logs are in `/tmp/mib2q-maint-141-*`; they are not
portable artifacts. Results and reproducible commands are preserved here/audit.

## Unavailable checks and limits

- Stock HMI base.jar, JCL and stock JDK are absent. Therefore Java transport,
  route-info/BAP, settings, PDC, lane-guidance full end-to-end and Java linkage/
  stock audits cannot complete; no stock substitutes were introduced.
- Docker contains no local images; the QNX cross-toolchain and JDK image are not
  present. No cross-build attempted or toolchain downloaded.
- Host FFmpeg development packages are not available through pkg-config; full
  H.264 host decoding/renderer execution is unavailable. The new policy test does
  not conceal this gap.
- macOS GL linking is unavailable on this Linux host. No successful browser
  screenshot or new visual inspection is claimed by these automated XML checks.
- Physical QNX signal semantics, stock receiver ABI, MOST/VC composition, real
  dual-decoder startup/fallback and adapter/phone behavior require hardware.
  Upstream's v1.4.1 release itself marks car validation pending.

## Files changed by this local increment

- New [tests/upstream_v141_test.py](../../tests/upstream_v141_test.py): meaningful
  upgrade, policy and preservation regressions, no product mocks or new interfaces.
- This maintenance report, plus project/status/change-ledger/agent/handoff/audit
  documentation describing the current maintenance scope and test authorization.
- No changes to the exact 20 upstream release files, dashboard sources/tests/
  previews, provider contract, five-phase roadmap or main/master branch.

## Before approving installation on this Audi A4 B9

These are **required evidence and a future test plan, not executed actions**.
Some v1.4.1 behavior can only be observed after a separately approved installation
or on a compatible bench unit; it cannot be proven on the unmodified car first.

Before installation approval: establish exact MHI2Q train/MU and hardware/cluster
variant, confirm upstream Phase 1 status, obtain independently authorized stock
HMI/receiver ABI evidence, confirm recoverable backups and build against that
unit's own HMI assets. Record baseline OEM navigation/VC operation and original
AAWireless firmware/behavior. An A4/year match or a TWO+ report is insufficient.

After separate authorization, validate on a bench or parked vehicle:

1. Galaxy S23 Ultra wired AA first, then the original AAWireless separately;
   cold-start connect before 120s, with/without a route, observe beyond 180s.
   No unplanned gal/session restart, black main screen, core hang or reboot.
2. Delay OFF: no unconditional 120s software phase; hardware startup follows
   main-screen quiet/yield timing. Delay ON: software until boot threshold, one
   protected switch to hardware, with the session retained. Preserve preference
   across program/menu upgrades and reboot; default remains OFF when absent.
3. Main-screen setup/config/start/stop/session teardown, reconnect and normal
   screen switches: cluster yields/reopens safely, native navigation remains
   usable, no unnecessary focus-triggered reconnects.
4. Controlled renderer FIN/RST/restart on a safe bench: receiver survives, stream
   resumes from a valid keyframe and stale frames/readiness are cleared. Do not
   deliberately hang the car's video core to test a watchdog.
5. Hardware-off/software fallback and recovery on a suitable bench: AA failure
   does not disable CarPlay; cooldown/watchdog behavior and MOST output are real
   hardware checks. Software AA performance may remain limited upstream.
6. CarPlay regressions with a separately available compatible iPhone: connect/
   reconnect, map/maneuvers, liveness, shared delayed preference and decoder yield.
   No iPhone/CarPlay vehicle result is claimed for the S23 setup.
7. Menu label/actions and backed-up uninstall/recovery on authorized equipment,
   including preserving user settings. Confirm OEM speed/warning instrumentation
   and guidance remain unobscured. Custom dashboards stay host-only/disabled.

## Review and rollback

Review locally against the original feature branch:

```sh
git diff feature/custom-dashboards...maintenance/upstream-v1.4.1
git log --oneline feature/custom-dashboards..maintenance/upstream-v1.4.1
```

No runtime change was made by this task, so there is no new vehicle rollback to
perform. The inherited installer backups/uninstall remain as upstream. Local
review was subsequently published to master with user approval. To undo this
maintenance increment after publication, use a reviewed revert of f5bd2e7 on a
separate branch, preserving later edits and dashboard work; do not reset or
force-push master. Future publication and releases require their own approval.
