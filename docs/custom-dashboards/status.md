# Project status

Status date: 2026-11-07. Scope: documentation-only, per the user's latest instruction.

## Completed

- Created a separate local project on `feature/custom-dashboards`, inheriting
  upstream's full code and Git history rather than using a reference submodule.
- Inspected upstream master at `b127988a68c8d690841f4a75d35717ffc01e668e`.
- Investigated maneuver/map renderers, common managed surfaces, AA hooks, Java
  lifecycle and data sources, deploy/mod scripts, GEM generation and host tests.
- Recorded confirmed, plausible/unverified and unknown conclusions with source links.
- Designed the modular host-only fallback and documented its safety boundary.
- Created an ordered implementation plan with commands, acceptance criteria and blockers.
- Added canonical AGENTS.md instructions, a continuation guide and entry files for
  Claude, Gemini and GitHub Copilot. They preserve the documentation-only scope
  and identify the first host increment for a later implementation request.

## Deferred

All new implementation code, renderer adapters, widget/data/config logic, previews,
new automated tests and performance checks. No dashboard runtime is supplied.
No changes were made to inherited upstream implementation files or to any vehicle.

## Git repository

The user supplied [alpersid-org/audi-mib2q-dashboards](https://github.com/alpersid-org/audi-mib2q-dashboards).
GitHub metadata confirms it is a fork of `kamgurgul/mib2q-carplay`. The inherited
`master` branch matches the investigated upstream baseline. Project documents
are maintained on `feature/custom-dashboards`; the original implementation and
README remain unchanged.

To retrieve the project:

```sh
git clone --branch feature/custom-dashboards https://github.com/alpersid-org/audi-mib2q-dashboards.git
cd audi-mib2q-dashboards
```

The upstream URL remains `https://github.com/kamgurgul/mib2q-carplay.git`; push
project changes only to the user-owned fork.

## Existing-test observations from investigation

The AI instruction update passed `git diff --check` and a host Python check of
relative links, heading anchors and code fences across all eight changed Markdown
documents (28 local links). Tool entry files all reference AGENTS.md. No runtime
regression tests were rerun for this documentation-only update.

These are preliminary environment observations, not a completed regression report:

- The upstream C suite was attempted before scope changed to documentation-only.
  Its first five named tests passed, then LeakSanitizer stopped at `rgd_lane_cache`
  because it could not operate under the environment's process tracing restrictions.
- A diagnostic retry with `ASAN_OPTIONS=detect_leaks=0` progressed through
  `signal_guard_test`, then failed an assertion in `bus_transport_test` at line 84.
  The cause was not established; this must not be presented as a dashboard defect,
  a passing regression suite or a confirmed sandbox socket failure.
- No new dashboard tests exist. Other Java/GL/QNX tests were not run after the user
  narrowed scope. Proprietary HMI assets and the project's stock JDK are absent.

## Open blockers for future vehicle work

1. Exact firmware/hardware and Phase 1 status are unconfirmed.
2. Original AAWireless compatibility is unknown; TWO+ evidence cannot be substituted.
3. A safe dashboard rectangle and ownership policy are not demonstrated.
4. Real signals during AA require verified availability, units and sample semantics.
5. Upstream bus-test failure needs reproduction/diagnosis before claiming a clean baseline.

The next implementation action, if separately requested, is PLAN step 1: the
isolated host module and mock snapshot contract. No vehicle integration is authorized.
