# Phase 2 requirements and safety boundary

## Target environment — supplied, not independently verified

- Audi A4 B9, 2017, 2.0 TDI quattro 190 HP.
- MMI High without touchscreen, Virtual Cockpit, physical rotary controller and buttons.
- Samsung Galaxy S23 Ultra with Android Auto.
- Original AAWireless adapter; **not AAWireless TWO+**.

Exact MHI2Q firmware, hardware compatibility and successful upstream Phase 1
installation are unknown. The repository's reported vehicle results do not
establish compatibility with this particular vehicle/phone/adapter combination.

## Current deliverable

A separate Git project containing source-backed investigation, proposed
architecture, requirements and an ordered implementation plan. Implementation,
previews and new automated tests are deferred at the user's request.

## Future MVP acceptance criteria

| Requirement | Acceptance evidence |
| --- | --- |
| Modular foundation | Dashboard manager, typed provider snapshots, widgets, config, layouts and renderer adapter have explicit boundaries. |
| Compact layout | Temperature, range, media and navigation candidates fit an explicit host viewport at readable sizes. |
| Extended layout | Adds trip and average consumption candidates without crowding or truncating units. |
| Data honesty | All initial values are visibly MOCK; unavailable, stale and invalid values cannot look like live zeros. |
| Existing sources only | A real adapter may use a demonstrated source only after units, validity, ownership and vehicle-specific meaning are verified. |
| Disabled default | Missing/invalid config and restore-default yield disabled Compact; selecting a layout alone never enables it. |
| Safe fallback | Provider failure, invalid geometry and unsupported layouts clear the dashboard frame. |
| Audi-inspired UI | Dark palette, high contrast, restrained accents, stable labels and no unnecessary animations. |
| Host validation | Unit/config/layout/rendering/fallback tests and a bounded performance measurement; reproducible previews. |
| Maintainability | Full upstream history, isolated changes and no unnecessary framework or dependency. |

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

Documentation-only release: remove the local project or revert its documentation
commit. No head-unit rollback is necessary. A future host prototype must support
disabled configuration and must not become part of upstream's installation payload
without a separately reviewed integration increment.
