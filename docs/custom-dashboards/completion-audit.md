# Phase 2 completion audit corrections — 2026-11-08

**Phase 2 remains NO-GO for closure.** The source corrections below are implemented
and host-tested; a safe VC renderer and actual vehicle acceptance are still
blocked by missing target evidence. This report supersedes earlier current-state
claims where the implementation changed; historical audit/review records remain.

## Scope and baseline

The user first requested an independent read-only audit, then: "After the audit
is done, don't stop until you fix everything. At the end, commit and push to
master". This authorizes corrective source, tests and fork master publication.
No vehicle access, installation, release publication or Phases 3–5 was performed.

Baseline: master `93e44079d6847eb84a5016f0f2e4fff852dbca1a`. The audit inspected the
recursive remote tree (563 files), retrieved 499 textual files, and reviewed all
dashboard/changed HMI code plus relevant inherited source, tests and history.
No checkout was initially attached, so uncommitted user changes were unavailable;
implementation uses a fresh clean clone and `fix/phase2-completion-audit`.
All 12 exposed branch heads were already in master's inspected history.

Authoritative requirements: [requirements.md](requirements.md), real vehicle
information inside the Virtual Cockpit, Compact/Extended, development-only mocks,
and omission of unsupported widgets. Host prototypes do not satisfy that outcome.

## Finding disposition

| Audit finding | Correction / current state | Evidence |
| --- | --- | --- |
| T01 conflicting JARs deleted without backup | Fixed: preserve `.carplay-stock` originals, refuse conflicting backups, restore on either uninstall path, refuse overwrite of an independently reinstalled JAR. | Installer `backup_once`, `check_conflict_restore`, `restore_conflicts`; `tests/installer_recovery_test.py`. |
| T02 false successful installation | Fixed: configuration patch/chmod/activation/sync failures propagate; errors trigger rollback rather than DONE. Setup no longer says nothing changed after a failure. | `mods/install_MoreIncredibleBash/mod/custom.sh`, `deploy/release/setup_custom.sh`. |
| T03 partial update leaves mixed files | Corrected for tested host failures: stage entire payload, snapshot affected paths, arm journal, activate, then commit. Errors/signals roll back; an abrupt termination retains explicit recovery. Target filesystem/power-loss behavior still unverified. | Installer `tx_save`, `tx_restore`, `tx_exit`, `recover`; ten recovery test methods. |
| G04 unsupported cards still rendered | Fixed on host: omit MISSING/not-supplied cards and reflow both layouts. Keep diagnostics for supplied stale/invalid values. No supplied widgets clears the frame (`no-data`). No mock fallback introduced. | `dashboard/layout.c:select_widgets`, `manager.c`, render/manager/vehicle tests. |
| T04 blocking/special file reads, poisoned temp path, restart behind old I/O | Hardened: C loaders reject symlinks/non-regular files, use nonblocking/no-follow opens (inode verification on platforms lacking O_NOFOLLOW). Java config rejects non-files/symlinks; writer creates unique regular staging files. A new generation can publish while cancelled old staging remains stalled; old generation cannot commit. Deletion failures log diagnostics. | `dashboard/{config,vehicle_provider}.c`, Java dashboard classes, CLI/FIFO/symlink and blocked-second-stage tests. |
| Writer first-read liveness | Hardened: stateful reader rejects records already older than 2 seconds, even on first/new-session observation. Stateless parse retains its documented 30-second record limit. | `db_vehicle_read`, `tests/dashboard_vehicle_test.c`. |
| T05 cached DSI readings get current read timestamps | Still UNVERIFIED, not changed blindly: these are change-driven subscriptions; a fixed expiration could discard legitimate unchanged readings. Need exact DSI validity/service-loss evidence; no claim that a bound handle proves live measurements. | `VehicleDataModule.readDashboard`, `Binding.isBound`; original signal verification gate. |
| T06 dashboard suites absent from CI | Fixed workflow wiring: C, ASan/UBSan, Java/feed, lifecycle, installer recovery and v1.4.1 regression steps. Fetch pinned upstream tags because the fork does not publish them. Remote workflow execution is separate from local validation. | `.github/workflows/tests.yml`. |
| T07 decoder yield continues after timeout | Preserved upstream policy; host timeout/SIGPIPE/yield tests pass. Do not modify the decoder behavior on speculation. Exact-target hardware validation/fallback remains required. | `aa_hook/aa_yield.c`, inherited tests and v1.4.1 policy fixtures. |

File I/O hardening is **not a hard real-time guarantee**. Regular-file metadata and
storage operations may still stall; Java 1.4 cannot atomically enforce no-follow
opens. Trusted target paths/permissions and actual QNX behavior require validation.
No old blocked thread is forcibly terminated; its unique temp file remains until
it finishes (or process restart), and generation checks forbid late publication.

Final review found and reproduced an additional normal-operation race in the
initial hardening: comparing lstat's inode with the opened record rejected valid
atomic publisher replacements. A concurrent writer/reader regression failed at
`rejected == 0`. With O_NOFOLLOW, fstat now validates the opened regular file without
requiring the previous inode; platforms lacking that flag retain the conservative
inode check. The concurrent regression performs 2,000 atomic publications against
3,000 reads. It is included in normal, sanitized and Java/C reader suites.

The formatter still exposes explicit UNAVAILABLE status to direct callers. Layouts
now omit those views; this is different from displaying a placeholder as a product
widget. Development MOCK previews with all fields supplied remain byte-identical.
Source values remain UNVERIFIED until actual signal acceptance; no source was
promoted to VERIFIED by host tests.

## Installer recovery contract

The new source stages under `/mnt/app/root/hooks/.carplay-transaction`, a private
persistent directory. It saves the active payload/configuration/conflict files
and the prior existence of original backups. This consumes additional temporary
storage; insufficient space fails staging before destination activation.

- Complete payload/snapshots and a counted journal are synchronized before arming.
- Catchable errors and HUP/INT/TERM restore the previous installation.
- A killed process or interrupted activation leaves the transaction directory.
  Another install or uninstall refuses until recovery is resolved.
- The install script's `recover` action restores an armed transaction; a committed
  transaction only needs journal cleanup. It does not require the SD payload.
- Recovery refuses if the recorded installer PID is live, or if the journal or a
  required snapshot is incomplete. It retains evidence on restoration failure.
- Conflicting navigation JARs are backed up before disabling them. Both uninstall
  paths restore the originals; an independently recreated destination is never
  overwritten silently.

This describes source behavior, **not authorization to run commands on the car**.
Before any separately authorized installation, validate the target filesystem,
free space, recovery access and exact release. Do not manually delete an armed
transaction or its snapshots. A PID reused by another live process causes a safe
refusal that needs human diagnosis. A storage corruption/power-cut guarantee on
QNX is not established by Linux process-termination tests.

## Validation on this executor

GCC, Python 3.12 and installed OpenJDK 21 compiler module; the `javac` launcher is
absent. `/tmp/mib2q-completion/bin/javac` calls the installed compiler module, as in
the prior final validation. No packages or proprietary assets were installed.

| Command/check | Result |
| --- | --- |
| `make -C dashboard all test test-sanitize test-java benchmark` with the temporary JAVAC launcher | PASS: six normal host suites, four ASan/UBSan/leak-checked C suites, Java config/feed/state and Java-to-C fixture. |
| `JAVAC=... bash scripts/test_app_lifecycle.sh` | PASS: all six isolated lifecycle scenarios. |
| `SKIP_JAR_CHECK=1 sh scripts/run_tests.sh` | PASS: 39 named inherited checks, including actual tee/SIGPIPE, yield, installer, GEM and lifecycle fixtures. Shipping-JAR portion explicitly skipped; inert fixture preload warnings expected. |
| `python3 tests/installer_recovery_test.py` | PASS: ten methods, dash/bash fixtures; normal/repeated install and both uninstall paths; staging/activation/config failures; killed/terminated update; failed rollback and recovery; committed cleanup; live-PID/truncated-journal refusal; symlink destination and restore collision. |
| `python3 tests/upstream_v141_test.py` after fetching upstream `v1.4` and `v1.4.1` locally | PASS: all four methods, including exact release blobs, hardware policy and upgrade/preference/backups/uninstall fixtures. |
| `bash scripts/test_install_payload.sh`, `test_install_dio.sh`, `test_install_listing.sh` | PASS on available dash/sh; payload rerun after final transaction changes. |
| Saved previews | PASS: existing SVG test compares all four against deterministic generated output; unchanged. |
| Target shipping Java/DSI/QNX, physical decoder/VC, actual vehicle resources | UNAVAILABLE: stock HMI/JCL/target toolchain, firmware/safe-surface evidence and vehicle absent. Host fixtures do not substitute. |
| Local ksh/mksh execution | UNAVAILABLE: neither installed; CI installs both and the same suites discover them. No claim of a local pdksh/QNX pass. |

Initial sanitizer execution inside the traced sandbox FAILED with LeakSanitizer's
explicit ptrace error. Rerunning outside tracing passed with all sanitizer options
at their defaults; no leak checking was disabled. Initial maintenance execution
FAILED because the fork clone lacked the upstream release tags; fetching the
actual upstream tags resolved it without weakening assertions. The initial new
installer suite exposed a fault-injection wrapper bug (it inspected cp's source
instead of its final destination); corrected wrapper now exercises real rollback.

Host benchmark: 20,000 preparations and 2,000 SVG exports/profile; preparation
2.04–2.68 us/frame, SVG 11.90–16.19 us/frame, manager 6,400 bytes, 30/42 commands.
These measurements exclude actual target graphics/storage/DSI costs.
Raw local logs: `/tmp/mib2q-completion/` (not committed or portable).

## Outstanding completion conditions

- [ ] Exact MHI2Q firmware/MU, receiver ABI, cluster variant, Phase 1 baseline and
  original AAWireless behavior established.
- [ ] Proven non-critical VC rectangle and ownership/OEM restoration policy for
  every supported mode, without speed/warning/navigation coverage.
- [ ] Actual signal availability, units, validity, DSI service-loss/cache freshness
  and session behavior validated; unavailable signals omitted.
- [ ] Native dashboard consumer/render loop, actual user controls and persistent
  configuration integrated after the safe-surface gate; QNX guard stays in place
  until that gate is satisfied. The existing Java gate remains session-bound.
- [ ] Full shipping Java 1.4/stock linkage and native target build/package verified.
- [ ] Actual vehicle acceptance, target resource measurements and installation/
  rollback validation, including 1.4.1 cold-start/two-minute/yield scenarios.

Supported CarPlay navigation and phone-free operation need an explicit support
contract; neither was invented as a mandatory feature by this audit. Multi-provider
resolution and AndroidDataProvider remain later-phase work, not Phase 2 blockers.
No Android companion, new phone transport or live companion telemetry was added.

**Next eligible action:** obtain target/signal/surface evidence. No safe graphics
routing can be selected from the current repository alone. Source corrections
and host passes do not make Phase 2 ready to close.
