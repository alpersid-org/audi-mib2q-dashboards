# P2-03: configuration and dashboard manager

PLAN step 3, host only. Implemented in
[config.c](../../dashboard/config.c), [manager.c](../../dashboard/manager.c) and
the API section of [dashboard.h](../../dashboard/dashboard.h). Two host CLIs:
[dashboard_config.c](../../dashboard/dashboard_config.c) and
[manager_preview.c](../../dashboard/manager_preview.c).

**Validation state:** strict C99 compilation (`make -C dashboard all`) succeeded.
No tests, demos, CLIs or sanitizers were executed: the user deferred testing to
the final stage. Behavior below is the implemented design, not verified behavior.
No GEM, upstream runtime, install payload or vehicle code was changed.

## Configuration format (version 1)

```text
# Custom dashboards host configuration (no vehicle output)
version=1
enabled=0
layout=compact
```

Rules (all enforced by `db_config_parse`):

- Whole input 1–512 bytes; each line at most 64 bytes; no NUL bytes.
- LF line endings; a trailing CR per line is tolerated. Last newline optional.
- Blank lines and lines starting with `#` (column 0) are ignored.
- Other lines are exactly `key=value` in printable ASCII with no spaces or tabs.
- The first key must be `version=1`. Other versions are rejected, not guessed.
- `enabled` is `0` or `1`; `layout` is `compact` or `extended`. Both optional,
  each at most once. Defaults: `enabled=0`, `layout=compact`.
- Unknown, duplicate or malformed keys reject the whole file.

Any rejection returns 0 and yields the **disabled Compact default**; a partial
file never enables the dashboard. `db_config_load_path` distinguishes a missing
file (`DB_CONFIG_MISSING`) from invalid content or a read error
(`DB_CONFIG_INVALID`); both yield the disabled default. `db_config_write` emits
the canonical form above after validating the struct.

## Actions

| Action | Effect |
| --- | --- |
| `db_config_enable` | `enabled=1`, layout unchanged |
| `db_config_disable` | `enabled=0`, layout unchanged |
| `db_config_select_layout` | Layout changed, **enablement unchanged** (selecting a layout never activates) |
| `db_config_restore_default` | Disabled Compact |

An action on an invalid struct (or an unknown layout) restores the default and
returns 0. These names match the candidate future GEM verbs in
[requirements.md](requirements.md); no GEM screen or button exists.

## Manager

`db_manager` holds a caller-supplied `db_provider`, the current config, one
snapshot and one bounded frame. It allocates nothing, starts no threads and
performs no I/O. The manager does not know which provider it holds: the vehicle
adapter, the mock, or a future AndroidDataProvider/composite all enter through
the same callback ([data-providers.md](data-providers.md)). The mock is selected
only by the host CLI.

`db_manager_update(now, width, height, area)` follows a fixed order:

1. Invalid config (rejected by `set_config`, until a valid config or explicit
   action) → disabled default, `OFF_CONFIG`; the provider is not read.
2. Disabled → `OFF_DISABLED`; **the provider is not read**.
3. Provider missing/failed → `OFF_PROVIDER`.
4. Layout rejects canvas/viewport → `OFF_GEOMETRY`.
5. Otherwise `READY` with the newly prepared frame.

Every non-READY outcome zeroes both snapshot and frame (count 0), so no previous
drawing survives. `set_config`, `disable`, `restore_default` and
`select_layout` also clear the frame immediately; the next update rebuilds it
only if the config is enabled. A layout switch therefore never shows an old
frame under the new layout, and never turns the dashboard on.

Status names (diagnostics): `disabled`, `invalid-config`, `provider-failure`,
`unsupported-geometry`, `ready`.

Not implemented here, by design: multi-provider resolution, provider identity
or session generation, persistence/atomic file replacement, and any consumer on
the head unit. The manager is single-threaded; a future integration owns locking.

## Host tools

```sh
make -C dashboard all
build/dashboard/dashboard_config show|enable|disable|compact|extended|restore-default [CONFIG] > new.cfg
build/dashboard/dashboard_manager_preview CONFIG wide|tall > frame.svg
```

`dashboard_config` reads CONFIG (missing → default, warning on stderr), applies
one action and prints the canonical result; it never writes files itself.
`show` exits 1 on invalid input; other actions recover from the default.

`dashboard_manager_preview` runs the manager with the explicit MOCK provider at
800x252 (wide) or 640x360 (tall). It writes SVG only when READY; otherwise it
writes nothing to stdout, prints the suppression reason and exits 3. Output still
carries MOCK badges and the host-only banner. `dashboard_preview` (P2-02) stays a
config-independent preview tool.

## Final-stage acceptance targets (deferred, not run)

- No config file and every malformed variant (empty, wrong/missing/late version,
  unknown/duplicate key, whitespace, bad value, NUL, >64-byte line, >512 bytes)
  → disabled default, manager emits zero draw commands.
- `compact`/`extended` actions and `db_manager_select_layout` keep `enabled`.
- Disabled manager never calls the provider (counting fake provider).
- Provider failure after a READY frame clears it; recovery rebuilds it.
- Undersized/invalid viewport after a READY frame clears it.
- Restore-default from any state → disabled Compact, zero commands.
- Config write/parse round trip; CLI exit codes 0/1/2/3 and empty stdout when
  suppressed. ASan/UBSan for parser bounds.

## Rollback

`make -C dashboard clean` removes the outputs including the two new tools.
Reverting the P2-03 commit removes the source. Nothing is installed; no
head-unit rollback is required.
