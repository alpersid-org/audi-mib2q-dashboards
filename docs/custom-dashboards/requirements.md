# Phase 2 requirements and safety boundary

**Current audit corrections (2026-11-08):** [completion-audit.md](completion-audit.md)
records installer recovery, widget omission, feed I/O/restart hardening and CI.
Host checks pass; firmware/safe-surface/stock-linkage/vehicle gates remain open.
Phase 2 is not complete. Earlier stage notes below retain their historical scope.

## Target environment — supplied, not independently verified

- Audi A4 B9, 2017, 2.0 TDI quattro 190 HP.
- MMI High without touchscreen, Virtual Cockpit, physical rotary controller and buttons.
- Samsung Galaxy S23 Ultra with Android Auto.
- Original AAWireless adapter; **not AAWireless TWO+**.

Exact MHI2Q firmware, hardware compatibility and successful upstream Phase 1
installation are unknown. The repository's reported vehicle results do not
establish compatibility with this particular vehicle/phone/adapter combination.

## Phase 2 scope — Real Vehicle Data Integration (authoritative)

User-defined scope, recorded 2026-11-07. It supersedes the earlier host-only,
mock-data MVP definition below wherever they conflict.

> The primary goal is to build fully functional custom dashboards displaying
> real data available from the vehicle's existing MMI systems.
>
> Requirements:
>
> 1. Investigate existing vehicle data acquisition mechanisms.
> 2. Identify which signals can be accessed safely and reliably.
> 3. Reuse existing data pipelines wherever possible.
> 4. Implement real-time data binding between available vehicle signals and
>    dashboard widgets.
> 5. Implement two functional dashboard layouts: Compact and Extended.
> 6. Use mock data only for automated testing and development.
> 7. Do not consider a dashboard displaying simulated data a completed Phase 2
>    deliverable.
> 8. If a particular vehicle signal is unavailable, document the limitation and
>    omit that widget until a verified data source becomes available.
>
> Do not introduce dependencies on a future Android companion application.
>
> The expected outcome is a working custom dashboard displaying actual vehicle
> information inside the Virtual Cockpit.

### Status against this scope (2026-11-07)

**Phase 2 is not complete.** P2-01..P2-05 delivered a host-only foundation
(provider interface, widgets, layouts rendered to SVG, config/manager, tests,
package) that runs only with mock data. Under requirement 7 that is not a
Phase 2 deliverable.

| # | Requirement | State |
| --- | --- | --- |
| 1 | Investigate acquisition mechanisms | Partial: source-level review in [analysis.md](analysis.md); nothing verified on this car. |
| 2 | Identify safe, reliable signals | Candidates only: outside temperature and range via `VehicleDataModule` (DSICarKombi; started for CarPlay sessions only), navigation distance/time via the AA trip feed / CarPlay RGD, media via existing now-playing DSI events. None verified on this A4 during Android Auto. |
| 3 | Reuse existing pipelines | P2-08 ([vehicle-provider.md](vehicle-provider.md)): VehicleDataModule (DSICarKombi) and the AA trip feed via AaRgState; no new DSI/CAN access. Not run on the car. |
| 4 | Real-time binding | P2-08: HMI publishes a DBV1 record every 500 ms; `db_vehicle_read` maps it to snapshots (badge UNVERIFIED). Media not supplied; CarPlay navigation not wired; no renderer consumes it yet (step 9). Unverified on the car. |
| 5 | Compact and Extended layouts | Host only (SVG); no Virtual Cockpit renderer. |
| 6 | Mock only for tests/dev | Partly: a real provider now exists (P2-08); the host preview tools still use mock by design (development). |
| 7 | Simulated ≠ deliverable | Hence incomplete. |
| 8 | Omit unverified widgets | Addressed in P2-07 ([widgets-p2-07.md](widgets-p2-07.md)): trip distance and average consumption removed; Extended redefined around the existing signal groups. Remaining widgets still need on-car verification. |

### Open decisions required from the user

1. **Head-unit code.** The expected outcome needs code that runs on the MMI and
   draws in the Virtual Cockpit. The current safety boundary (host only; no
   head-unit execution, installation or new hooks) forbids that. It stays in
   force until the user explicitly changes it. The user, not an agent, would
   install and test on the car.
2. **Display surface.** Only display layer 98 (maneuver graphics) and layer 99
   (phone map video) are proven paths into the cluster. A dashboard area must
   be chosen and validated on the car, never covering speed, warnings or
   navigation guidance.
3. **Vehicle evidence.** Exact MHI2Q firmware/hardware, original AAWireless
   behaviour and per-signal availability/units while Android Auto runs.

If no safe surface can be demonstrated, the host foundation remains useful,
but it must be reported as a blocked Phase 2, not a completed one.

## Earlier deliverable definition (superseded where it conflicts)

A separate Git project containing source-backed investigation, proposed
architecture, requirements and an ordered implementation plan, now extended by
user-authorized P2-01 data/widgets and P2-02 Compact/Extended host layouts,
SVG export and four mock previews. Configuration/activation remains later work.
Testing was deferred to the final stage, which P2-04 has started; every change
must still be documented; see changes.md and the audit.

## Host-foundation acceptance criteria (earlier MVP; still apply to the foundation)

Phase 2 is designed within the complete [five-phase roadmap](../ROADMAP.md).
The required common [Data Provider Interface](data-providers.md) must support
Phase 2 VehicleDataProvider and later AndroidDataProvider through the same
Dashboard Manager and Widget System. The standalone Galaxy S23 Ultra app belongs
to Phase 3; communication belongs to Phase 4, dashboard consumption to Phase 5.
Documenting these boundaries is required; implementing them early is not.

| Requirement | Acceptance evidence |
| --- | --- |
| Modular foundation | Dashboard manager, typed provider snapshots, widgets, config, layouts and renderer adapter have explicit boundaries. |
| Compact layout | Temperature, range, media and navigation candidates fit an explicit host viewport at readable sizes. |
| Extended layout | Earlier: adds trip and average consumption. **Superseded (P2-07):** temperature, range, now playing, distance to destination, time to destination, destination. |
| Data honesty | Mock values are visibly MOCK and confined to tests/development (scope req. 6); unavailable, stale and invalid values cannot look like live zeros. |
| Existing sources only | A real adapter may use a demonstrated source only after units, validity, ownership and vehicle-specific meaning are verified. |
| Disabled default | Missing/invalid config and restore-default yield disabled Compact; selecting a layout alone never enables it. |
| Safe fallback | Provider failure, invalid geometry and unsupported layouts clear the dashboard frame. |
| Audi-inspired UI | Dark palette, high contrast, restrained accents, stable labels and no unnecessary animations. |
| Host validation | Unit/config/layout/rendering/fallback tests and a bounded performance measurement; reproducible previews. |
| Maintainability | Full upstream history, isolated changes and no unnecessary framework or dependency. |
| Provider evolution | Generic callback/normalized snapshots; transport-independent widgets; explicit source identity, freshness and selection when live/multiple providers are introduced. |

## Non-negotiable boundaries

- Do not alter braking, steering, ADAS, vehicle control, warning logic or other
  critical systems.
- Never replace or obscure speed, warning lights or safety-critical instrumentation.
- Preserve OEM functionality and navigation guidance.
- Do not intercept or repurpose safety-critical vehicle controls.
- Keep dashboard features disabled by default.
- Do not flash, install, deploy to the vehicle or execute commands on the head unit.
- Do not destructively modify stock files or introduce an invasive/unproven hook.
- Treat compilation as host evidence only, never vehicle compatibility evidence.
- Do not implement DPF monitoring, an Android companion app or new Android-to-MMI
  communication. Roadmap Phases 3–5 are outside scope.

## Vehicle integration gate

If no safe non-critical VC rectangle can be established, stop vehicle integration
and retain the standalone host renderer plan. Neither a free displayable ID nor
the phone map's SafeArea proves such a rectangle exists. Existing 98/99 surfaces
serve navigation; their demonstrated coexistence does not authorize a dashboard.

GEM controls may be designed using upstream's menu generator, but must not be
implemented until a safely gated consumer exists. Candidate future controls are
Enable, Disable, Compact, Extended and Restore Default.

## Rollback

P2-01 host release: `make -C dashboard clean` removes generated outputs; stop
using the text demo. Source changes can be reverted by their feature commit. No
head-unit rollback is necessary. A future host prototype must support
disabled configuration and must not become part of upstream's installation payload
without a separately reviewed integration increment.
