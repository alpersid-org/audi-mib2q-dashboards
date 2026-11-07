# P2-07: verified-source widgets only

PLAN step 7, host only. User request: "yes" to starting step 7 after the Phase 2
scope was recorded ([requirements.md](requirements.md#phase-2-scope--real-vehicle-data-integration-authoritative)).

**Validation state:** `make -C dashboard all` compiled cleanly (strict C99,
`-Werror`). That is a build step only; nothing was executed. No tests were run
or edited, per the user's instruction "do not run any test, there will be a ask
at the end to implement and run the tests". **The existing test suites are now
out of date and will fail**; see the list below.

## What changed and why

| Change | Reason (scope requirement) |
| --- | --- |
| Removed `DB_TRIP`, `DB_CONSUMPTION`, `db_snapshot.trip`, `db_snapshot.consumption`, the `db_consumption` type and their mock values | No existing pipeline supplies trip distance or average consumption ([analysis.md](analysis.md) data inventory). Req. 8: omit until a verified source exists. |
| Added `DB_NAV_DISTANCE`, `DB_NAV_TIME`, `DB_DESTINATION` | Split the existing navigation group (Android Auto trip feed `aa_hook/aa_trip.h`; CarPlay RGD) into separate cards for Extended. All three read the **same atomic navigation sample**, so they always describe one route. |
| Extended = temperature, range, now playing, distance to destination, time to destination, destination | Extended now shows more detail from the same verified signal groups, not extra unverified signals. Compact is unchanged: temperature, range, now playing, navigation. |
| `-1` meters/seconds and an empty destination mean "not supplied" | Matches `aa_trip_t` (`-1 = not supplied`). Split cards show `--` / UNAVAILABLE for a missing part instead of INVALID. The Compact navigation card shows whichever of distance/time exists and is UNAVAILABLE only when both are missing. Values outside the bounds remain INVALID. |
| Time card: minutes rounded up; `N min` below one hour, else `H h MM min` | Never optimistic; no ETA clock is derived. |
| Deleted `dashboard/previews/extended-wide.svg` and `extended-tall.svg` | They displayed the removed mock widgets. Regeneration is deferred with the tests (no preview generation under the current instruction). Compact previews are unaffected by this change. |
| Mock provider comment: tests/development only | Req. 6. |

Signals available in existing pipelines but **deliberately not shown**: vehicle
speed, fuel warning, reverse gear and standstill (`VehicleDataModule`). Speed
and warnings must never be duplicated or obscured by the dashboard (safety
boundary); gear/standstill are not dashboard information.

Note: upstream already forwards Android Auto final-destination distance and ETA
to the stock VC route display ([README](../../README.md), android-auto overview).
The Extended navigation cards therefore duplicate data the stock cluster can
show; the dashboard must not cover that stock display.

## Widget contract (current)

| Widget | Source group | Bounds (host guards) | Display |
| --- | --- | --- | --- |
| Outside temperature | `temperature` | -1000..1000 (0.1 C) | `12.5 C` |
| Driving range | `range` | 0..10,000,000 m | whole km, rounded down |
| Now playing | `media` | terminated, nonempty title | title, optional artist |
| Navigation (Compact) | `navigation` | each part -1 or in range | `42.5 km / 38 min`, or the supplied part; destination as detail |
| Distance to destination | `navigation` | -1 or 0..1,000,000,000 m | `42.5 km` |
| Time to destination | `navigation` | -1 or 0..31,536,000 s | `38 min`, `1 h 05 min` |
| Destination | `navigation` | terminated; empty = not supplied | name, ASCII, elided |

## Tests now out of date (fix at the final testing stage)

Not edited or run, per the user's instruction:

- `tests/dashboard_test.c` uses `DB_TRIP`, `DB_CONSUMPTION`, `s.trip`,
  `s.consumption`: **will not compile**. Its navigation cases expect `-1`-style
  values to be INVALID and need the new not-supplied semantics.
- `tests/dashboard_render_test.c` expects `5.6 L/100km`, `84.3 km` and the
  combined `42.5 km / 38 min` text in Extended.
- `tests/dashboard_svg_test.py` expects `42.5 km / 38 min` in all four profiles
  and compares saved Extended previews (now deleted).
- `tests/dashboard_manager_test.c` `ready()` uses `84.3 km` to tell Extended
  from Compact.
- New cases are needed for the split navigation widgets, `-1`/empty handling
  and the `H h MM min` format; Extended previews must be regenerated.

Rollback: revert the P2-07 commit (restores the widgets and previews).
