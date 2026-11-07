# Audi MIB2Q — Custom Dashboards project

Repository: [alpersid-org/audi-mib2q-dashboards](https://github.com/alpersid-org/audi-mib2q-dashboards),
a GitHub fork of the original project. It includes the full source and Git history of
[kamgurgul/mib2q-carplay](https://github.com/kamgurgul/mib2q-carplay), inspected at
commit `b127988a68c8d690841f4a75d35717ffc01e668e` (1.4.1).

**Phase 2 scope: Real Vehicle Data Integration** — working Compact/Extended
dashboards in the Virtual Cockpit with real MMI data; mock only for tests. See
[requirements](docs/custom-dashboards/requirements.md#phase-2-scope--real-vehicle-data-integration-authoritative).
**Phase 2 is not complete.** Available final host checks pass, including the
vehicle-feed/Java-C and corrected shutdown regressions; see
[final validation](docs/custom-dashboards/validation-final.md). Full stock linkage,
firmware/ABI, safe VC rendering and actual vehicle validation remain outstanding.

**History: P2-02, host Compact/Extended layouts and SVG export.**
P2-01 data/widgets are complete. Step 2 adds bounded frames, a mock preview CLI
and four saved SVG artifacts (P2-07 later removed the two Extended ones).
P2-03 added host config/manager; P2-04 ran the host tests (historical results).

**Current state (review fixes on top of 312215a):** P2-07 removed unbacked widgets;
P2-08 added the VehicleDataProvider. P2-08 **modifies four inherited HMI Java files**
(`CarPlayApp`, `VehicleDataModule`, `AaRgState`, `AaBridge`) and adds
`java_patch/com/luka/carplay/dashboard/`. `scripts/build_java.sh` compiles every
`java_patch` source into the shipping `carplay_hook.jar`, so a normal Java build now
ships the dashboard feed; it stays inactive unless `/mnt/app/carplay_dashboard.cfg`
enables it. Native hooks, renderers, installers and the original
[upstream README](README.md) are unchanged. The C dashboard renderer side remains
host-only (QNX guard). No vehicle connection, installation or deployment is performed.

Published branch: `master`, tracking the fork's `origin/master`. All dashboard
steps 1 and 2, roadmap/provider design, agent audit and maintenance work are included.
The user explicitly authorized this consolidation on 2026-11-07.

Current maintenance review: [upstream v1.4.1](docs/maintenance/upstream-v1.4.1.md)
was prepared on `maintenance/upstream-v1.4.1` and is now included in `master`.
The exact release was already inherited; this task verified it and added upgrade
regression coverage without modifying runtime/dashboard code. Maintenance host
tests were explicitly authorized and completed as reported. Publication does not
start another implementation task or authorize a release or vehicle action.

**Long-term context:** follow the [five-phase roadmap](docs/ROADMAP.md). The future
Galaxy S23 Ultra app starts independently in Phase 3, communicates with MMI in
Phase 4 and supplies existing widgets in Phase 5. Phase 2's VehicleDataProvider
role and future AndroidDataProvider share one normalized interface/manager/widget
system; see [provider design](docs/custom-dashboards/data-providers.md).
VehicleDataProvider is implemented with UNVERIFIED provenance and host checks;
AndroidDataProvider and Android communication remain unimplemented.

## Documents

| Document | Purpose |
| --- | --- |
| [Five-phase roadmap](docs/ROADMAP.md) | Persistent full-project context, phase boundaries and acceptance targets. |
| [v1.4.1 maintenance review](docs/maintenance/upstream-v1.4.1.md) | Full release mapping, existing integration proof, new regressions, results and hardware gates. |
| [Data Provider Interface design](docs/custom-dashboards/data-providers.md) | Vehicle/Android adapter roles, shared manager/widgets and future metadata/selection rules. |
| [AI agent instructions](AGENTS.md) | Canonical scope, safety, engineering and validation rules for agents. |
| [AI continuation guide](docs/custom-dashboards/HANDOFF.md) | Restart checklist, next eligible task and reusable prompt. |
| [Current agent checkpoint](docs/custom-dashboards/audit/CURRENT.md) | Active task, partial work, validation and exact next action for cross-agent resume. |
| [Agent audit trail](docs/custom-dashboards/audit/README.md) | Mandatory checkpoint protocol and append-only event history. |
| [Host module API and commands](docs/custom-dashboards/module.md) | Implemented P2-01 contracts, units, limits, tests and text demo. |
| [Reviewable increment package](docs/custom-dashboards/package.md) | P2-05 quick start, artifacts, test outcomes, limitations/blocker, change summary and rollback. |
| [Host layouts and previews](docs/custom-dashboards/rendering.md) | P2-02 SVGs, API, typography, constraints and deferred final checks. |
| [Change ledger](docs/custom-dashboards/changes.md) | Per-file record of every P2-02 change. |
| [Architecture](docs/custom-dashboards/architecture.md) | Modular host prototype design and future integration boundary. |
| [Repository investigation](docs/custom-dashboards/analysis.md) | Confirmed capabilities, plausible but unverified options, and unknowns with source references. |
| [Executable Phase 2 plan](docs/custom-dashboards/PLAN.md) | Ordered tasks, commands, acceptance criteria and blocked work. |
| [Requirements and safety](docs/custom-dashboards/requirements.md) | Vehicle context, candidate widgets, scope and mandatory safety rules. |
| [Status](docs/custom-dashboards/status.md) | Completed documentation, deferred implementation and open technical blockers. |
| [Attribution](docs/custom-dashboards/NOTICE.md) | Upstream attribution and licensing. |

## Intended environment

Audi A4 B9 (2017), 2.0 TDI quattro 190 HP, MMI High without touchscreen, Virtual
Cockpit and physical MMI controls. Phone: Galaxy S23 Ultra with Android Auto and
the original AAWireless adapter, not TWO+.

Exact firmware, hardware compatibility, original AAWireless behavior and successful
Phase 1 installation have not been independently confirmed. No spare, safe
dashboard rectangle has been demonstrated. The implementation
therefore remains a host-only renderer with mock values. No safe vehicle
integration is claimed.

Only roadmap Phase 2 is in scope. Android companion applications, new
Android-to-MMI communication and DPF monitoring are excluded.

## Repository structure

The upstream code is part of this repository itself, with its history; it is not
merely linked or included as a submodule. New project documents are isolated under
`docs/custom-dashboards/`. See PLAN.md there before beginning any implementation.

To obtain the complete project:

```sh
git clone --branch master https://github.com/alpersid-org/audi-mib2q-dashboards.git
cd audi-mib2q-dashboards
```

The fork's `master` contains the full upstream baseline, project documents,
host module and maintenance verification. The earlier feature and maintenance
branches remain available; no history was rewritten.

Build with `make -C dashboard all`; generate mock SVGs with
`make -C dashboard previews`. Test commands (`make -C dashboard test test-sanitize`)
are listed for the final stage only. Tests are deferred: the user said "do not run any test, there will be a ask at the end to implement and run the tests". The suites are out of date since P2-07/P2-08; run them only at that final request.

AI agents should start with AGENTS.md. Claude, Gemini and GitHub Copilot entry
files point to the same instructions; other tools can load them explicitly.
