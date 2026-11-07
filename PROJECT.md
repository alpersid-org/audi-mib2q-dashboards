# Audi MIB2Q — Custom Dashboards project

Repository: [alpersid-org/audi-mib2q-dashboards](https://github.com/alpersid-org/audi-mib2q-dashboards),
a GitHub fork of the original project. It includes the full source and Git history of
[kamgurgul/mib2q-carplay](https://github.com/kamgurgul/mib2q-carplay), inspected at
commit `b127988a68c8d690841f4a75d35717ffc01e668e` (1.4.1).

**Current increment: P2-02, host Compact/Extended layouts and SVG export.**
P2-01 data/widgets are complete. Step 2 adds bounded frames, a mock preview CLI
and four saved SVG artifacts. Configuration/activation remains step 3. The user
has deferred all further testing until the final stage; each change is documented.
Earlier test results are historical evidence, not final validation. Inherited runtime files and the original [upstream README](README.md)
are unchanged; no vehicle connection, installation or deployment is performed.

Development branch: `feature/custom-dashboards`.

## Documents

| Document | Purpose |
| --- | --- |
| [AI agent instructions](AGENTS.md) | Canonical scope, safety, engineering and validation rules for agents. |
| [AI continuation guide](docs/custom-dashboards/HANDOFF.md) | Restart checklist, next eligible task and reusable prompt. |
| [Current agent checkpoint](docs/custom-dashboards/audit/CURRENT.md) | Active task, partial work, validation and exact next action for cross-agent resume. |
| [Agent audit trail](docs/custom-dashboards/audit/README.md) | Mandatory checkpoint protocol and append-only event history. |
| [Host module API and commands](docs/custom-dashboards/module.md) | Implemented P2-01 contracts, units, limits, tests and text demo. |
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

To work with the documentation branch:

```sh
git clone --branch feature/custom-dashboards https://github.com/alpersid-org/audi-mib2q-dashboards.git
cd audi-mib2q-dashboards
```

The fork's `master` branch retains the inherited upstream baseline. The
`feature/custom-dashboards` branch contains project documents and the host module.

Build with `make -C dashboard all`; generate mock SVGs with
`make -C dashboard previews`. Tests are reserved for the final stage under the
current user instruction; do not run the test targets now.

AI agents should start with AGENTS.md. Claude, Gemini and GitHub Copilot entry
files point to the same instructions; other tools can load them explicitly.
