# P2-01: host data and widget module

**Current audit corrections (2026-11-08):** [completion-audit.md](completion-audit.md)
records installer recovery, widget omission, feed I/O/restart hardening and CI.
Host checks pass; firmware/safe-surface/stock-linkage/vehicle gates remain open.
Phase 2 is not complete. Earlier stage notes below retain their historical scope.

Implemented in [dashboard/](../../dashboard/) with tests in
[dashboard_test.c](../../tests/dashboard_test.c). This page describes PLAN step 1: a library and deterministic text demo.
P2-02 now adds [host layouts and SVG export](rendering.md). It creates no display window, connects to no
service, and changes no inherited runtime, install payload or firmware file (true
for P2-01/P2-02; P2-08 later modified inherited HMI Java, see
[vehicle-provider.md](vehicle-provider.md)).

## Testing policy

The P2-02 test deferral ended when the user started P2-04, the final testing
stage. P2-04 adds config/manager and CLI suites and a benchmark target; results
are in [validation.md](validation.md). P2-01 historical results remain recorded; see status.md.
**Current P2-FINAL:** the final test request superseded that deferral. Suites
are repaired and pass; see [validation-final.md](validation-final.md). The Java
feed has host tests with external-service fixtures; stock/vehicle checks remain
unavailable. There are seven widget formatters in the current demo API.

## Build and run

From the repository root, using a host C compiler, `ar` and make:

```sh
make -C dashboard all test demo
make -C dashboard test-sanitize
```

The first command compiles strict C99 (`-Wall -Wextra -Werror -pedantic`), creates
`build/dashboard/libdashboard.a`, runs the tests, and prints the six mock widgets.
The second runs the same tests with AddressSanitizer and UndefinedBehaviorSanitizer
(including default leak detection). No SDK, HMI jar or new package is needed.
Build files are ignored by Git. `make -C dashboard clean` removes only this
module's generated outputs. Sanitizer availability depends on the host compiler.

In this managed environment LeakSanitizer fails under the process-traced sandbox.
The same sanitizer command passed outside that sandbox without disabling any
sanitizer check. Do not interpret the sandboxed invocation as a passing test.

The demo header says `HOST PROTOTYPE - MOCK DATA - NO VEHICLE OUTPUT`; examples:

```text
OUTSIDE TEMPERATURE  [MOCK] 12.5 C
DRIVING RANGE        [MOCK] 620 km | Estimated range
NOW PLAYING          [MOCK] Night Drive | Studio Session
NAVIGATION           [MOCK] 42.5 km / 38 min | City Centre
DISTANCE TO DESTINATION [MOCK] 42.5 km | Remaining route
TIME TO DESTINATION  [MOCK] 38 min | Estimated by navigation
DESTINATION          [MOCK] City Centre
```

This text output is a data/formatter demonstration, not a dashboard screenshot.
The listing reflects the P2-07 widget set as designed; it was not re-executed.
Host SVG layouts are supplied separately by P2-02. P2-03 adds a host
configuration/manager ([config.md](config.md)); no vehicle activation exists. Further tests are deferred to the final stage
by the user; commands on this page are retained for that stage.

## Snapshot and provider contract

See [data-providers.md](data-providers.md) for VehicleDataProvider (Phase 2) and
future AndroidDataProvider (Phases 4–5). Both target this same callback/snapshot
boundary and common manager/widgets. Only db_mock_read is implemented today.
Provider identity/session metadata and multi-source selection are planned
extensions; current MOCK/VERIFIED provenance is not vehicle/Android identity.

[dashboard.h](../../dashboard/dashboard.h) exposes C types and a C-compatible
function interface. Snapshots own their fixed arrays; no heap allocations or
borrowed string pointers are used. They are in-process structs, not a network
format, stable binary ABI, firmware adapter or protection from malicious callbacks.

Each widget datum has availability (`MISSING`, `AVAILABLE`, `INVALID`), source
provenance and a caller-supplied monotonic sample time in milliseconds. Zero is a
valid clock epoch. Every caller/provider must use the same clock domain. UTC or
system wall time must not be substituted without explicit adapter conversion.

`db_snapshot_read` initializes a candidate snapshot and calls a synchronous
provider. Exactly `1` means success; any other result, missing provider or missing
callback clears previous caller data. Partial successful snapshots leave omitted
fields missing. It does not poll or spawn threads. A provider must return promptly
and synchronize its own source; there is no callback timeout in this increment.

`db_mock_read` fills deterministic example values, all marked MOCK, stamped at the
supplied `now_ms`. Time changes affect sample metadata only. These are fresh
synthetic values on each call, not evidence of actual sensor freshness.
`DB_SOURCE_VERIFIED` is reserved for a later validated source; no such provider
ships. Do not relabel a mock as verified in product code.

## Widget formatting contract

| Widget | Canonical input | Host validation bounds | Display policy |
| --- | --- | --- | --- |
| Temperature | 0.1 degrees C | -1000 through 1000 | One decimal, including `-0.1 C` |
| Driving range | Meters | 0 through 10,000,000 | Whole km, rounded down |
| Media | Title/artist, each 96-byte capacity | Both terminated, nonempty title | Title plus optional artist |
| Navigation | Final-destination meters, seconds, destination | each -1 (not supplied) or 0–1,000,000,000 m / 0–31,536,000 s; terminated name | Supplied parts; duration rounded up to minutes; optional destination |
| Distance / time to destination, destination (P2-07) | Same navigation sample | As above; empty name = not supplied | See [widgets-p2-07.md](widgets-p2-07.md) |

Bounds are deliberate prototype input guards, not verified firmware/sensor limits.
Trip distance and average consumption were removed in P2-07 (no source). All
real signal meanings remain unverified on this car.
Navigation is an atomic group: a future adapter must take the oldest contributing
sample time so fresh destination text cannot revive old estimates. Since P2-07
a part the source did not supply (-1 / empty) renders as UNAVAILABLE. No ETA clock is inferred.

`db_widget_format` always replaces the output. Missing data produces `--` and
`UNAVAILABLE`; invalid values/provenance produce `INVALID`; timestamps after now
produce `INVALID TIME`; age greater than 30,000 ms produces `STALE`. Exactly
30,000 ms is accepted. The threshold is a host policy, not a verified source rate.
Failure outputs have empty detail strings, preventing stale text from surviving.
Available zero is valid; a zeroed snapshot is missing and cannot imply live zero.

Output has fixed-capacity label/value/detail/badge arrays, a machine-readable
status and provenance. Source labels are MOCK/VERIFIED only for valid values;
error badges describe the failure while the source field retains known provenance.
Unknown availability/source enum values fail closed.

The prototype text contract is ASCII. Control/non-ASCII bytes become `?`; all
input arrays must contain a terminator within their capacity. A future renderer
must escape output for its destination (for example XML for SVG). This formatter
does not perform markup escaping, font shaping or production Unicode support.

## Future integration points and limits

The header reserves a 48-command rectangle/text frame using upstream `cr_rect_t`
from [visible_area.h](../../maneuver_render/visible_area.h). No upstream crop
fallback is called. Zeroed frames contain no drawing commands. P2-02 implements host frame building
and SVG export; see rendering.md. The header rejects QNX compilation until a
separate safe-surface decision authorizes a vehicle backend.

The module is not included in upstream build/release scripts. It has no vehicle enable
switch; the explicit SVG CLI produces host files only. Disabled-by-default
configuration and rendering suppression are PLAN step 3 acceptance targets.
There is no performance claim for the head unit and no vehicle compatibility
claim. Two Compact SVG artifacts exist under dashboard/previews/ (Extended removed in
P2-07); final visual acceptance is deferred by the user.

Rollback is `make -C dashboard clean` to remove outputs and ceasing host demo use;
no head-unit file restoration is required. Source changes can be reverted with
their feature commit, preserving unrelated work.
