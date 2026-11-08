# P2-C07: settings persistence and live application (host part)

Completion-plan work package [P2-C07](completion-plan.md#p2-c07--controls-configuration-and-persistence).
User request: "start C07". Format and typed mapping: [settings-contract.md](settings-contract.md).

## Done (host)

| Item | Implementation |
| --- | --- |
| Atomic persistence | `db_config_store_path` ([config.c](../../dashboard/config.c)): canonical text to `<path>.tmp.<pid>` (`O_CREAT|O_EXCL`), flush + `fsync`, `rename`; on any failure the old file stays byte-identical and the temporary file is removed. Invalid configs are refused. |
| CLI write-back | `dashboard_config -w enable\|disable\|compact\|extended\|restore-default CONFIG` applies one action and stores atomically (`show` cannot be combined with `-w`). Layout actions keep enablement; restore-default gives disabled Compact; an invalid file recovers from the default. |
| Live settings in the runtime | `db_runtime_load_settings` (bounded ≤ 512-byte read) applies the file through the idempotent reconciler; call it every controller cycle. Missing → disabled default; invalid/partial/unknown version → **fails closed** (disabled, `OFF_CONFIG`). |
| Applied vs persisted | `db_runtime_persisted_settings` (status + config as read) is distinct from `db_runtime_applied_config` (what the runtime acts on). They differ e.g. for an invalid file, or after a shutdown. |
| Shutdown latch kept | After `db_runtime_shutdown`, per-cycle reloads only record the intent; only an explicit `db_runtime_apply_config` restarts output. |
| No reconnect for disable/layout | C: a disabled file releases output inside `load_settings` itself (same cycle); a layout change redraws on the next tick without re-acquiring. Java: `DashboardModule` re-reads the config every 500 ms cycle; disabled or invalid → stops publishing and withdraws its record; re-enabled in the same session → resumes with the same writer session and increasing sequence. |
| Reboot gate | An old enabled file cannot bypass the gates: without a supported profile the runtime stays `unsupported`, and in production mode (no ACCEPTED signal) it stays `waiting`. |

### Measured latency (host)

| Path | Result |
| --- | --- |
| C runtime: disabled file → output released | within the same `load_settings` call (0 extra ticks) |
| Java: disabled file → record withdrawn | **505 ms** in `DashboardModuleTest.liveSettings` (one 500 ms writer period; test bound 1100 ms) |

These are host measurements. The target budget on the unit is not agreed yet;
on-unit behaviour is confirmed in C09.

## Not done here, and why

- **GEM menu buttons** (Enable, Disable, Compact, Extended, Restore Default via
  `scripts/build_gem_menu.sh` / `deploy/gem_menu/run.sh`): the plan and
  `AGENTS.md` allow them only **after a safely gated consumer exists**. There is
  no on-unit consumer until C06, so adding buttons now would advertise a function
  with no validated backend. `dashboard_config -w` is the building block the
  buttons will call.
- **Enabling mid-session (Android Auto):** `VehicleDataModule` binds `DSICarKombi`
  only at session start when the dashboard is enabled. Enabling during a session
  therefore needs the next phone connection for temperature/range; disabling and
  layout changes do not. Starting/stopping the binding mid-session is stock-linked
  lifecycle work that cannot be compiled or tested here (stock JAR absent).
- **Wheel/safety keys:** not used (plan rule).

## Validation (2026-11-08, host)

| Check | Result |
| --- | --- |
| `dashboard_settings_test` (new; also under ASan/UBSan) | **PASSED** |
| `dashboard_cli_test.py` with `-w` cases | **PASSED** |
| `DashboardModuleTest.liveSettings` (new) | **PASSED** (505 ms) |
| Clean `make -C dashboard all test test-sanitize test-java`; `test_app_lifecycle.sh` | **PASSED** |
| Mutants: reload ignores the shutdown latch; publisher never withdraws | both **caught** |
