# P2-C05: shared dashboard settings contract (v1)

User request: "start C05 settings contract". This is the single specification for
the dashboard configuration file shared by the HMI (Java) and the dashboard
runtime (C). Both parsers must agree on every byte sequence; the shared vectors
enforce that.

| Side | Type | Parser | File |
| --- | --- | --- | --- |
| C | `db_config` (`version`, `enabled`, `layout`) | `db_config_parse` / `db_config_load_path` | [config.c](../../dashboard/config.c) |
| Java | `DashboardSettings` (`valid`, `enabled`, `layout`) | `DashboardConfig.parse` / `load` | [DashboardSettings.java](../../java_patch/com/luka/carplay/dashboard/DashboardSettings.java), [DashboardConfig.java](../../java_patch/com/luka/carplay/dashboard/DashboardConfig.java) |
| Shared tests | 48 conformance vectors | both suites | [dashboard_config_vectors.txt](../../tests/fixtures/dashboard_config_vectors.txt) |

On the unit the file is `/mnt/app/carplay_dashboard.cfg` (`DashboardConfig.PATH`).

## Format

- 1–512 bytes; each line at most 64 bytes (excluding LF; one trailing CR per line
  tolerated); no NUL byte anywhere, comments included.
- Blank lines and lines starting with `#` in column 0 are ignored.
- Every other line is exactly `key=value`, printable ASCII, no spaces or tabs.
- The first key must be `version=1`. Then, each at most once and in any order:
  `enabled=0|1` (default `0`), `layout=compact|extended` (default `compact`).
- Anything else (unknown, duplicate or malformed key; other version; BOM; CR used
  as a line separator; oversize input or line) makes the **whole file invalid**.

## Typed meaning

| Input | C `db_config` | Java `DashboardSettings` |
| --- | --- | --- |
| valid file | parse returns 1; `version=1`, `enabled`, `layout` as written | `valid=true`, `enabled`, `layout` as written |
| invalid, missing, unreadable, directory, FIFO, symlink | returns 0 / not LOADED; disabled Compact default | `DEFAULT`: `valid=false`, `enabled=false`, `layout=COMPACT` |

- Layout values are equal on both sides: `DB_COMPACT = LAYOUT_COMPACT = 0`,
  `DB_EXTENDED = LAYOUT_EXTENDED = 1`.
- An invalid file never enables the dashboard and its layout is never honoured.
- Selecting a layout never enables the dashboard (`enabled` stays as written).
- `DashboardConfig.enabled()` is exactly `load().enabled`.
- Changing the format means a new `version`, new vectors, and both parsers
  updated in the same commit. v1 parsers reject other versions.

## Lifecycle (reconciliation)

The Java side reads the file when its modules start (phone session start), so a
change takes effect at the next session (session-bound, see vehicle-provider.md).
The C runtime applies settings through the idempotent `db_runtime_apply_config`:
an identical config is a no-op, disabling clears and releases output at once,
and a layout change redraws without re-acquiring. Per-signal eligibility is
**not** part of this file: it comes from the compiled acceptance register
([signal-acceptance.md](signal-acceptance.md)), so a config file can never mark
data as VERIFIED.

## Validation (2026-11-08, host)

| Check | Result |
| --- | --- |
| `dashboard_config_vectors_test` (C, 48 vectors) | **PASSED** |
| `DashboardConfigTest` (Java, same 48 vectors + typed load/default/file-type checks) | **PASSED** |
| Clean `make -C dashboard all test test-sanitize test-java` | **PASSED** |
| Mutants: Java case-insensitive `layout`; C line cap 65 | both **caught** by the shared vectors |

A C mutant allowing the space byte survived: it is equivalent, because exact
key/value matching already rejects every vector containing a space.
