# P2-05: reviewable Phase 2 host increment

> **Scope note:** under the user's Phase 2 scope ("Real Vehicle Data
> Integration", [requirements.md](requirements.md#phase-2-scope--real-vehicle-data-integration-authoritative))
> this package is a mock-only foundation, **not** the Phase 2 deliverable.

PLAN step 5. This page is the entry point for a reviewer or developer who wants
to clone the fork, build the host prototype, run its tests and inspect the
previews **without any vehicle access**. It packages P2-01 to P2-04.

**Testing state for this step:** nothing was run for P2-05 (no build, test,
demo, benchmark or preview generation), per the user's instruction "do not run
any test, there will be a ask at the end to implement and run the tests". All
results quoted below come from P2-04 (2026-11-07) and are historical evidence.
Only read-only `git` inspection was used to prepare this page.

## What the increment is

A standalone C99 host library and tools in [dashboard/](../../dashboard/):

| Part | Step | Contract |
| --- | --- | --- |
| Typed snapshots, `db_provider` callback, mock provider, widget formatters | P2-01 | [module.md](module.md) |
| Compact (2x2) / Extended (3x2) layouts, bounded draw list, SVG export | P2-02 | [rendering.md](rendering.md) |
| Strict v1 config (disabled by default), actions, Dashboard Manager | P2-03 | [config.md](config.md) |
| Host test suites, sanitizer target, bounded benchmark, regression report | P2-04 | [validation.md](validation.md) |

All displayed values are MOCK. The header refuses QNX compilation. Nothing is
installed, and the upstream install payload does not include this module.

## Quick start (host only)

Prerequisites: a C99 compiler, `ar`, `make`, Python 3 (standard library only).
No QNX SDK, stock HMI files, JDK or network access is needed.

```sh
git clone https://github.com/alpersid-org/audi-mib2q-dashboards.git
cd audi-mib2q-dashboards
make -C dashboard all          # library, demo, preview and config/manager tools
make -C dashboard demo         # text demo of the six mock widgets
make -C dashboard previews     # regenerate the four saved SVGs (identical bytes expected)
```

Config and manager tools ([config.md](config.md)):

```sh
build/dashboard/dashboard_config enable > on.cfg          # enabled Compact
build/dashboard/dashboard_config extended on.cfg > ext.cfg # layout keeps enabled=1
build/dashboard/dashboard_manager_preview ext.cfg wide > frame.svg
build/dashboard/dashboard_manager_preview missing.cfg wide # exit 3, no output (disabled)
```

Tests (run them only when the user asks for the final testing stage):

```sh
make -C dashboard test           # five suites
make -C dashboard test-sanitize  # ASan/UBSan
make -C dashboard benchmark      # host timing only
SKIP_JAR_CHECK=1 sh scripts/run_tests.sh   # inherited upstream host checks
```

`make -C dashboard clean` removes every generated output, including test fixtures.

## Artifacts

| Artifact | Location | State |
| --- | --- | --- |
| Compact/Extended previews, 800x252 and 640x360 | [dashboard/previews/](../../dashboard/previews/) | Saved in Git. P2-04 confirmed they are byte-identical to CLI output; no code changed since (git diff 85c523e..HEAD on dashboard/ and tests/ is empty). |
| Manager-gated preview | `dashboard_manager_preview` output | Not saved; generated on demand from a config file. |
| Regression report | [validation.md](validation.md) | P2-04 results and unavailable checks. |

## Test outcomes (historical, from P2-04)

- Dashboard: `test` and `test-sanitize` passed. That covers the five suites,
  including config/manager and CLI tests, with ASan/UBSan and leak detection.
- Benchmark on that host: prepare 1.0–1.5 us/frame, SVG 8.4–11.4 us/frame.
  Extended uses 42 of 48 draw-command slots. These numbers say nothing about the
  head unit.
- Upstream `run_tests.sh`: the official run failed at check 22 (gcc 13
  `warn_unused_result` in upstream altscreen code under `-Werror`). With two
  labelled diagnostic adjustments, 38 of 39 checks passed; `hw301_bridge_tests`
  needs IPv6, which that container lacked.
- Seven Java/stock scripts were unavailable (no proprietary stock HMI JAR or
  JDK 8); the native maneuver script passed its Linux part, but its macOS GL
  link is unavailable on Linux.

## Limitations and the integration blocker

The prototype is complete as a host increment. It is **not** a vehicle feature:

- **No safe display surface is known.** No non-critical Virtual Cockpit
  rectangle has been demonstrated. Displayables 98/99 belong to navigation;
  a free ID or phone SafeArea is not proof. This is the blocker for PLAN step 6.
- **Vehicle facts unconfirmed:** exact MHI2Q firmware/hardware, receiver ABI,
  original AAWireless (not TWO+) behaviour, and Phase 1 installation success.
- **No live data:** only the mock provider exists. VehicleDataModule is
  CarPlay-only in the investigated baseline. Multi-provider resolution and
  provider identity/session metadata are designed but not implemented
  ([data-providers.md](data-providers.md)).
- **No activation path:** no GEM controls, no renderer on the head unit, no
  config file location on the unit. Host dimensions are not certified VC sizes.
- **Known follow-ups:** draw-command headroom (42/48) before adding widgets; the
  upstream gcc 13 `unused-result` build break (upstream code, needs approval);
  final visual/readability review.

Roadmap Phases 3–5 (standalone Android app, transport, Android data in
dashboards) are not started ([ROADMAP.md](../ROADMAP.md)).

## Change summary relative to upstream

Read-only check at master f7e9fb2 (the P2-05 base):
`git diff --name-status b127988a68c8d690841f4a75d35717ffc01e668e f7e9fb2` lists
48 paths, **all added**; P2-05 adds this page as a 49th. No inherited upstream file was modified or deleted
(the upstream README, runtime, hooks, renderers, scripts and install payload are
untouched). The added paths:

| Area | Added paths |
| --- | --- |
| Agent/project entry | `AGENTS.md`, `CLAUDE.md`, `GEMINI.md`, `.github/copilot-instructions.md`, `PROJECT.md` |
| Dashboard source | `dashboard/{Makefile,dashboard.h,provider.c,mock_provider.c,widgets.c,layout.c,svg.c,config.c,manager.c}` |
| Dashboard tools | `dashboard/{demo.c,preview.c,dashboard_config.c,manager_preview.c,benchmark.c}` |
| Previews | `dashboard/previews/{README.md,compact-wide.svg,compact-tall.svg}` (Extended SVGs removed in P2-07) |
| Tests | `tests/{dashboard_test.c,dashboard_render_test.c,dashboard_svg_test.py,dashboard_manager_test.c,dashboard_cli_test.py,upstream_v141_test.py}` |
| Documentation | `docs/ROADMAP.md`, `docs/maintenance/upstream-v1.4.1.md`, and 16 files under `docs/custom-dashboards/` including `audit/` (17 with this page) |

Per-file change history: [changes.md](changes.md). Session evidence:
[audit/LOG.md](audit/LOG.md).

## Rollback

- Host: `make -C dashboard clean`; stop using the tools. Nothing else to undo.
- Head unit: nothing was installed, flashed or executed, so there is no
  head-unit rollback.
- Source: because every change is an added file, removing the module means
  reverting the relevant commits (reviewed `git revert`, preserving later
  work). Never reset or force-push `master`.

## Acceptance (PLAN step 5)

| Criterion | State |
| --- | --- |
| Previews saved in `dashboard/previews/` | Met (four SVGs; verified in P2-04; unchanged since) |
| Reproduction commands, test outcomes, limitations documented | Met (this page, validation.md) |
| Changed files and rollback summarized | Met (sections above) |
| Inherited implementation unchanged vs b127988 | Met by read-only git inspection: no M/D entries |
| Clone/build/test/inspect without vehicle access | Documented; not re-executed in P2-05 (user's no-test instruction) |
