# P2-C04: signal acceptance register

Completion-plan work package [P2-C04](completion-plan.md#p2-c04--real-source-acceptance-capabilities-and-freshness).
User request: "start C04". This register is the source of truth for which signals
a production dashboard may show. The compiled copy is
`db_signal_policy_default()` in [signal_policy.c](../../dashboard/signal_policy.c);
change both together, with evidence.

## States and modes

| State | Production mode (default) | Validation mode (explicit, on-car checks) |
| --- | --- | --- |
| **OMITTED** | hidden | hidden |
| **CANDIDATE** | hidden | shown, badge **UNVERIFIED** |
| **ACCEPTED** | shown, promoted to **VERIFIED** for that signal only | shown, VERIFIED |

MOCK and UNKNOWN sources are removed in both modes. A provider can never assert
VERIFIED: promotion comes only from this register, per signal, never from a
global flag. A removed group is zeroed, so no old value can be revived.
**No signal is ACCEPTED today**, so the default (production) runtime draws nothing.
That is intended: requirement 7 forbids treating unverified data as the deliverable.

## Register

| Field | Outside temperature | Driving range | Navigation (final destination) | Now playing |
| --- | --- | --- | --- | --- |
| State | **CANDIDATE** | **CANDIDATE** | **CANDIDATE** | **OMITTED** |
| Source API / event | `DSICarKombi.updateBCOutsideTemperature` (attr 65) via `VehicleDataModule` | `updateBCTotalCurrentRange` (attr 69), else `updateBCCurrentRange1` (attr 8) | AA 0x8006/0x8007 → `aa_hook/aa_trip.c` → `/tmp/aa_trip` (AAT1) → `AaTripFeed` → `AaRgState.onTrip` | none in the repo for Android Auto (stock `DSIAndroidAuto2` fields unavailable) |
| Meaning | ambient temperature from the cluster BC | estimated remaining range; **total vs range-1 meaning on this diesel unverified** | remaining distance/time to the **final destination** (0x8007 destination distances), not next-turn distance; destination name from 0x8006 | — |
| Raw unit → canonical | °C or °F (`temperatureUnit`) → 0.1 °C | km or mi (`distanceUnit`) → km (rounded), published as m | metres / seconds as sent (-1 = not supplied) → same | — |
| Validity flags | DSI `valid == 1`; `CarBCTemperature.state` **not** trusted (unset on the upstream car) | DSI `valid == 1` and `distanceValueState`/`state == BCVALUESTATE_VALID` | AAT1 magic + FNV checksum, `-1` for absent parts, route active, sample after route start | — |
| Expected update behaviour | change-driven DSI notification (value may stay unchanged for long periods) | change-driven | phone-app dependent; Waze/Maps differ; ~per guidance update | — |
| Sample / confirmation age | DBV1 stamps the read time while the Kombi binding is held; record age ≤ 30 s, writer liveness 2 s | same | oldest of destination/estimate sample time; 30 s freshness in `AaRgState` | — |
| Session / route ownership | any phone session (module lifecycle); not tied to a route | same | Android Auto session, active route only; cleared on route end/session reset | — |
| Service-loss behaviour | **not established**: `Binding.isBound()` reflects a held handle, not live DSI health | **not established** | route end / session reset / stale feed → absent; writer death → record stale | — |
| Host evidence | conversion code reviewed; DBV1 + reader tests | same | `AaRgStateDashboardTest`, `aa_trip` tests, reader tests | — |
| **Evidence still required to ACCEPT** | on-car log of DSI value/valid/state vs the cluster display, unit switching (°C/°F), behaviour when the BC service restarts | same, plus which attribute this A4 diesel provides and whether it matches the cluster's range | logs with Google Maps and Waze showing final-destination values against the phone, absent-field behaviour, route end | a verified AA media source (stock class survey) |

Signals present in existing pipelines but **excluded by design**: vehicle speed,
fuel warning, reverse gear, standstill (safety boundary / not dashboard data).
Removed in P2-07: trip distance, average consumption (no source).
**UNKNOWN** (handover B5): RPM, coolant/oil temperature, boost, gear position,
accelerations, TPMS — need the `lsd.jxe` DSI survey before any can become a candidate.

## Plan items and status

| Plan item | Status |
| --- | --- |
| 1. Per-signal table | **Done** (this register). |
| 2. Conversions; total vs range-1; final-destination semantics | Conversions reviewed in code; final-destination meaning confirmed in code (0x8007). **Range attribute meaning on this car: needs vehicle logs.** |
| 3. DSI cache assumption, notification/confirmation tracking | **Blocked on C02 evidence** (stock DSI behaviour, service restarts). Not guessed: no new freshness rule invented. |
| 4. Invalidation on service loss / session / route / clock / writer death | Route end, session reset, clock (future/epoch), writer death and session change: implemented and tested. **DSI service loss: blocked on C02** (no reliable stock API knowledge). |
| 5. Per-signal/profile eligibility, no global VERIFIED | **Done**: `signal_policy.{h,c}`, runtime integration, tests. DBV1 unchanged (no new semantics needed). |

## Validation (2026-11-08, host)

`make -C dashboard clean all test test-sanitize test-java`: **PASSED**, including
the new `dashboard_signal_policy_test` and the updated runtime test (now explicit
VALIDATION mode for lifecycle cases, plus a production-default case that stays
waiting with a valid record). Mutants caught: global VERIFIED promotion,
candidates allowed in production, MOCK allowed.

## How a signal becomes ACCEPTED

1. Capture the evidence in the row above (user-run logs; no agent vehicle access).
2. Record it here with the log reference and date; set the state to ACCEPTED.
3. Change `db_signal_policy_default()` in the same commit and extend the policy
   test. Never accept a signal by configuration.
