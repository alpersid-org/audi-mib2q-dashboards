# P2-C05: dashboard runtime controller (host)

Completion-plan work package [P2-C05](completion-plan.md#p2-c05--consumer-and-lifecycle-controller-on-the-host).
User request: "start C05". Code: [runtime.h](../../dashboard/runtime.h),
[runtime.c](../../dashboard/runtime.c); tests:
[dashboard_runtime_test.c](../../tests/dashboard_runtime_test.c).

The controller drives **DBV1 record → `db_vehicle_read` → manager → frame →
output backend**. The backend is a contract; the only implementation is the
host test fake. There is still no native/QNX backend (`dashboard.h` refuses QNX),
no displayable or context id, and nothing runs on the vehicle.

## States

| State | When | Output |
| --- | --- | --- |
| `disabled` | config off (default), invalid config, shut down | cleared and released |
| `unsupported` | no profile, or a layout rejects its geometry | cleared and released |
| `waiting` | enabled, but no accepted data (missing, stale, invalid record; writer silent > 2 s; no supplied widget) | cleared and released |
| `active` | accepted frame shown on an owned surface | drawn |
| `recovering` | acquire or draw failed | cleared and released; retry after backoff |

Every non-active path goes through the same `leave()` step: clear (if something
was shown), then release (if owned). The OEM composition is therefore restored
whenever the dashboard is not actively drawing.

## Rules

- **One persistent reader, no mock path.** The runtime embeds one stateful
  `db_vehicle_source`; there is no API to inject another provider. Mock data
  exists only in tests and host preview tools.
- **Data before ownership.** Each tick reads the record once (the only file I/O),
  and the surface is acquired only when an accepted frame exists. Draw callbacks
  receive a ready frame and do no I/O.
- **Dirty-only drawing.** A frame is drawn when it differs from the last drawn
  frame, or after re-acquisition. Unchanged data costs no backend call.
- **Bounded recovery.** Acquire/draw failures back off 250 ms, doubling to an 8 s
  cap; success resets it. No retry happens before the deadline.
- **Complete backend required.** Acquire, draw, clear and release callbacks must
  all exist before acquisition. An incomplete backend enters recovering without
  invoking any output callback, including during retries, disable or shutdown.
- **Latched shutdown.** Shutdown clears/releases output, disables runtime and
  manager configuration, and cancels pending retries. Later ticks stay disabled
  without reading records or invoking the backend. Explicitly applying enabled
  configuration restarts immediately; the selected layout is preserved.
- **Reconciler.** `db_runtime_apply_config` is idempotent; disabling clears and
  releases immediately; a layout change redraws on the next tick without
  re-acquiring. `db_runtime_set_profile` releases on a geometry change (the surface
  must be re-acquired for new geometry) and ignores an identical profile.
- **Profiles are test inputs.** A profile is supported only if both layouts can
  lay out every card (worst-case probe snapshot, discarded after the check). This
  says nothing about a real VC rectangle.
- The caller drives `db_runtime_tick` at a fixed cadence (e.g. 250 ms) from one
  thread; the struct must not move after init.

## Validation (2026-11-08, host)

| Check | Result |
| --- | --- |
| `make -C dashboard all test test-sanitize test-java` (clean build) | **PASSED**, including `dashboard_runtime_test` under ASan/UBSan |
| Mutation: always redraw / keep surface while waiting / acquire before data | each **caught** by the runtime test; source restored |

Covered: all five states; data-before-ownership; dirty redraw; layout, profile and
config reconciliation (including idempotence and invalid config); stale writer →
release, new writer session → re-acquire; corrupt record; bounded exponential
backoff and its cap; draw failure clearing a partial draw; idempotent shutdown;
a backend without callbacks; every granted surface released.

Review corrections (2026-11-08): `make -C dashboard all test` and
`make -C dashboard test-sanitize` **PASSED**, including regressions for repeated
ticks with fresh records after shutdown, explicit restart, shutdown during
recovery, and all 15 incomplete callback combinations. Sanitizers ran outside
the traced sandbox with default leak checks enabled. Java was unchanged and
remains unavailable on this host (no `javac`); the original Java result above is
Claude's historical evidence. See [resolution](reviews/claude-p2-c05-resolution.md).

## Signal eligibility (P2-C04)

Every record read passes `db_signal_policy_apply` before the manager. Default:
the compiled register in PRODUCTION mode, so nothing is drawn until a signal is
ACCEPTED; `db_runtime_set_signal_policy` selects VALIDATION for on-car checks.
See [signal-acceptance.md](signal-acceptance.md).

## Not done here

Native backend (C06), the real surface/context mapping (C03), signal acceptance
(C04), MMI controls and shared typed settings with Java (C07), packaging (C08) and
vehicle acceptance (C09). The Java publisher's own lifecycle is unchanged.
