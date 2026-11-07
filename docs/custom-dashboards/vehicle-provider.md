# P2-08: VehicleDataProvider (real-time binding)

PLAN step 8 (scope requirements 3–4). User request: "merge to master and start
step 8". This step connects the dashboard to data the HMI **already receives**,
through existing pipelines. It adds no new DSI attributes, CAN access, sockets or
renderer commands, and it never controls the vehicle.

**Review fixes (2026-11-07).** Codex's review of 312215a found R1–R5; R1, R2, R3,
session invalidation, the Android Auto subscription gap and R5 are implemented
and untested; R4 (Java test harness) waits for the final test request. See the
[resolution record](reviews/claude-312215a-resolution.md).

**Validation state.** Nothing was executed (user instruction: "do not run any
test, there will be a ask at the end to implement and run the tests"). The C code
compiles with strict C99 (`make -C dashboard all`, build step only).
`DashboardConfig.java` compiles standalone with the host `javac` at Java 8 level
(1.4 is no longer accepted by modern `javac`; no post-1.4 features are used).
The other Java changes **cannot be compiled here**: they link against stock HMI
classes (DSI, OSGi) that are not in the repository. **Nothing has run on the car.**
Installing and checking it is the user's step 10.

## Data path

```
DSICarKombi ─► VehicleDataModule ─┐   (outside temperature, range; existing bindings)
                                  ├─► DashboardModule ─► /tmp/carplay_dashboard (DBV1, 500 ms)
AA gal hook ─► /tmp/aa_trip ─►    │                                   │
  AaTripFeed ─► AaRgState ────────┘   (destination, distance, time)   ▼
                                                    db_vehicle_read ─► db_snapshot ─► Dashboard Manager
```

| Part | File | Role |
| --- | --- | --- |
| Enable gate | `java_patch/com/luka/carplay/dashboard/DashboardConfig.java` | Reads `/mnt/app/carplay_dashboard.cfg`, the same strict v1 format as `dashboard/config.c`. Only a valid file with `enabled=1` enables the feed; missing/invalid means disabled. |
| Publisher | `java_patch/com/luka/carplay/dashboard/DashboardModule.java` | New module, started for any phone session. If enabled, writes the 256-byte DBV1 record every 500 ms (per-generation temp file, then rename). Each start is a new writer generation; an ended generation can never publish, and no record survives `stop()` (lifecycle below). If disabled, does nothing and removes any old record. |
| Vehicle readings | `java_patch/com/luka/carplay/core/VehicleDataModule.java` | Now started for Android Auto sessions too, but **binds during Android Auto only when the dashboard is enabled**, and then **only `DSICarKombi`** with no sender thread (temperature/range are all the dashboard uses). CarPlay sessions keep the original three bindings and `CMD_VEHICLE`. New read-only `dashboardVehicle()`. |
| Session list | `java_patch/com/luka/carplay/core/CarPlayApp.java` | `VehicleDataModule` owner changed from CarPlay to any session; `DashboardModule` appended (any session). |
| Navigation | `java_patch/com/luka/carplay/aa/AaRgState.java`, `AaBridge.java` | Read-only `dashboardNav()`: the destination, distance and time that `onTrip` already resolved (route-start and 30 s freshness guards), plus the destination sample time. Route state is not changed. |
| Reader | `dashboard/vehicle_provider.c` | `db_vehicle_read` / `db_vehicle_parse`: validates the record and fills `db_snapshot` with provenance `DB_SOURCE_VEHICLE`. |
| Provenance | `dashboard/dashboard.h`, `widgets.c` | New `DB_SOURCE_VEHICLE`, badge **UNVERIFIED** until on-car validation (step 10) promotes a signal to `VERIFIED`. |

## Signals

| Widget | Source | State |
| --- | --- | --- |
| Outside temperature | `DSICarKombi.updateBCOutsideTemperature` via VehicleDataModule | Bound; not yet seen on this car during Android Auto. |
| Driving range | `BCTotalCurrentRange`, else `BCCurrentRange1` | Bound; km precision (VehicleDataModule rounds to km); total-vs-range-1 meaning on this diesel unverified. |
| Distance / time to destination, destination | Android Auto trip feed (`aa_hook/aa_trip.c` → `AaRgState`) | Android Auto sessions only; depends on the navigation app sending rich trip data. |
| Now playing | — | **Not supplied.** During Android Auto it is only reachable through stock `DSIAndroidAuto2` classes whose fields are not available in this repository; writing it would mean guessing signatures. The card shows UNAVAILABLE. Scope requirement 8 says to omit such a widget; layout-level omission is deferred to P2-09, where the real Virtual Cockpit layout is defined. |
| Trip distance, average consumption | — | Removed in P2-07 (no source). |
| Speed, warnings, gear | Available in VehicleDataModule | **Deliberately excluded** (safety boundary). |

Navigation during **CarPlay** sessions (CarPlay RGD) is not wired yet; the
record leaves the navigation group empty there.

## Time and freshness

- DSI reports temperature and range **on change**. As in the existing CarPlay
  snapshot, a valid value counts as current while the DSI subscription is bound,
  so `DashboardModule` stamps it with the read time. An invalid or unbound value
  is absent.
- Navigation uses `AaRgState`'s resolved values. The group's sample time is the
  **oldest** contributing part (destination name, estimate), so a fresh name
  cannot revive an old estimate.
- **Write time last (R1):** the writer reads every source first and only then
  takes the record's write time, so no sample time is newer than it. The reader
  still rejects a record with a sample newer than its write time (corruption).
- All record times are `System.currentTimeMillis()`. The C reader compares them
  with `CLOCK_REALTIME` on the same unit, converts each to an age, and maps it
  onto the manager's own clock. Samples from the future are INVALID; the widget
  formatter applies the 30 s limit.
- **Reader clock after the read (R3):** `db_vehicle_read` samples the wall clock
  only after the whole record is in memory, so a record published while the file
  was being opened is not judged "future". Ages are measured slightly late and
  mapped onto the caller's earlier `now_ms`, which errs towards older.
- **Writer death suppresses the dashboard:** a missing, malformed, future or
  >30 s old record makes the provider fail, and the manager clears its frame.
- **Writer liveness and session identity:** each record carries a non-zero writer
  session (new for every start and HMI process) and a sequence starting at 1.
  With a `db_vehicle_source` context the reader tracks them: a new session is
  accepted and tracked; within a session the sequence must not go backwards; and
  if the same (session, sequence) is still the newest after 2 s
  (`DB_VEHICLE_WRITER_TIMEOUT_MS`, four writer periods) the provider fails. A dead
  HMI therefore blanks the dashboard within about 2 s instead of 30 s.

## Writer lifecycle (review R2)

- `start()` opens a new generation; the writer thread is bound to it.
- At most one publication is in flight. It may begin only for the current
  generation; file I/O runs outside the lock, so `stop()` never waits for I/O.
- `stop()` bumps the generation and removes the record, unless a publication is
  in flight. That publication re-checks after its rename, sees the ended
  generation and removes its own record. No newer generation can publish in the
  meantime, so an old writer never replaces a newer generation's record.
- Temporary files are per generation (`/tmp/carplay_dashboard.tmp.<gen>`).
- The only file operations under the lock are the small deletes that must be
  ordered with publication.

## DBV1 record (256 bytes, big-endian)

| Offset | Field |
| --- | --- |
| 0 | magic `DBV1` (0x44425631) |
| 4 | sequence u32, starts at 1 per writer session |
| 8 | written ms u64 |
| 16 | temperature, 0.1 C, i32 (`INT32_MIN` = none) |
| 20 | range, metres, i32 (-1 = none) |
| 24 / 28 | destination metres / seconds, i32 (-1 = not supplied) |
| 32 / 40 / 48 | temperature / range / navigation sample ms, u64 (0 = absent) |
| 56 | session owner u8: 0 none, 1 CarPlay, 2 Android Auto |
| 57–59 | zero |
| 60 | writer session u32, non-zero (new per start and per HMI process) |
| 64–191 | destination, UTF-8, NUL-terminated, ≤ 95 bytes (cut on a character boundary) |
| 192–251 | zero |
| 252 | FNV-1a u32 over bytes 0–251 |

The reader rejects: wrong size or magic, bad checksum, non-zero padding, owner >
2, zero session or sequence, unterminated or over-long destination, sample times
after the write time, and a stale or future record; with a context also a
backwards sequence or a writer silent for more than 2 s.

## Limitations

- **No data without a phone session.** The modules start only when CarPlayApp
  activates a CarPlay or Android Auto session. Temperature and range need no
  phone, but starting them outside sessions would change the module lifecycle;
  that is left for a separate decision.
- **Enabling and disabling are session-bound.** The config is read when the
  modules start. Creating, editing or deleting it mid-session takes effect at
  the next phone connection; a running session keeps publishing until it ends.
- **Owner type is not session proof.** The reader cannot tell which phone session
  is current; it relies on removal at `stop()` and the writer session/sequence
  liveness above.
- **No consumer yet.** Nothing on the unit calls `db_vehicle_read` until the
  renderer of step 9 exists. `dashboard.h` still refuses QNX compilation.
- **Unverified on this car:** DSI availability, units, update behaviour while
  Android Auto runs, `/tmp` and `/mnt/app` write/read behaviour, and timing.
- **CarPlay behaviour change to check:** VehicleDataModule now starts in Android
  Auto sessions (no-op unless enabled). CarPlay output is unchanged by design.

## Tests needed at the final stage (none written; user instruction)

- `db_vehicle_parse`: valid record, every rejection rule, INT32_MIN/-1 absent
  parts, clock mapping (future sample, age beyond the clock epoch, 30 s
  boundary), UTF-8 destination handling, `db_vehicle_read` missing/short/long file.
- Java (needs the stock JAR/JDK 8): `DashboardConfig` parity with `config.c`
  rules, `DashboardModule.encode` byte layout and checksum, UTF-8 truncation,
  oldest-time rule, `publish` replace semantics, VehicleDataModule gating per
  session owner, `AaRgState.dashboardNav` before/after route end.
- A cross-language fixture: a Java-encoded record parsed by the C reader.
- Manager integration with `db_vehicle_read` (stale writer clears the frame).

Additional cases from the review: a clock tick between source reads and the
write time (R1); stop during collection, write and rename, and immediate restart
(R2); a publication between file open and clock sample (R3); session change,
backwards sequence and the 2 s writer timeout; Kombi-only binding in Android Auto.
The Java lifecycle harness also needs the R4 repair (isolated `DashboardModule`
fixture and owner/gating assertions).

Rollback: revert the P2-08 and review-fix commits. On the unit, a missing config
file keeps the feature off. Deleting `/mnt/app/carplay_dashboard.cfg` disables it
from the next phone session; the current session continues until it ends.
