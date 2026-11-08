# Common Data Provider Interface

**Current audit corrections (2026-11-08):** [completion-audit.md](completion-audit.md)
records installer recovery, widget omission, feed I/O/restart hardening and CI.
Host checks pass; firmware/safe-surface/stock-linkage/vehicle gates remain open.
Phase 2 is not complete. Earlier stage notes below retain their historical scope.

Required architecture: **VehicleDataProvider in Phase 2**, then
**AndroidDataProvider in Phases 4–5**, eventually feed the **same Dashboard Manager
and Widget System**. The future Android app runs independently on the Galaxy S23
Ultra in Phase 3 before MMI communication is introduced. See [ROADMAP](../ROADMAP.md).

This document defines the target design. It adds no adapter or transport code.
The later no-test instruction ended with P2-FINAL; current host results and
remaining stock/vehicle gaps are in [validation-final.md](validation-final.md).

## What exists and what is planned

| Element | Evidence / implementation state |
| --- | --- |
| `db_provider { read, context }` | **Confirmed by code:** [dashboard.h](../../dashboard/dashboard.h). Synchronous callback independent of transport. |
| Owned typed `db_snapshot` and `db_sample` | **Confirmed by code:** fixed-size fields, canonical units, availability, MOCK/VERIFIED provenance and monotonic sample time. |
| Snapshot read boundary | **Confirmed by code:** [provider.c](../../dashboard/provider.c) initializes candidate data and clears output on callback failure. |
| MockDataProvider role | **Confirmed by code:** [mock_provider.c](../../dashboard/mock_provider.c), exposed as `db_mock_read`. It is not a live VehicleDataProvider. |
| Existing widgets/layouts | **Confirmed by code:** [widgets.c](../../dashboard/widgets.c) and [layout.c](../../dashboard/layout.c) consume normalized snapshots without knowing the producer. |
| VehicleDataProvider | **Implemented in P2-08, unverified on the car:** HMI `DashboardModule` record + C `db_vehicle_read` ([vehicle-provider.md](vehicle-provider.md)). Provenance `DB_SOURCE_VEHICLE` (badge UNVERIFIED). |
| Dashboard Manager / provider resolution | **Manager implemented (P2-03):** single injected `db_provider`, config gate, frame clearing ([config.md](config.md)). **Multi-provider resolution not implemented**; can stay absent until needed. |
| AndroidDataProvider | **Future, unimplemented:** normalized adapter over a validated received-data cache, introduced with Phases 4–5. No transport selected. |
| Provider identity, session generation and configurable selection | **Partly implemented for the vehicle feed:** `DB_SOURCE_VEHICLE` provenance; the DBV1 record carries session-owner type, a per-start writer session and a sequence, and `db_vehicle_read` tracks writer liveness ([vehicle-provider.md](vehicle-provider.md)). No general per-provider origin field in `db_sample` and no configurable selection. |

## Interface and ownership

Preserve the existing `db_read_fn(context, now_ms, out_snapshot)` boundary.
The same manager can receive a VehicleDataProvider, a development mock, or later
a composite provider implementing that callback. Concrete provider names denote
roles; they do not require a C++ class framework, reflection or new service bus.

Each provider publishes an owned fixed-capacity snapshot of semantic signals.
Provider-specific raw data, object pointers, connection handles and protocols
stay behind `context`. Callbacks must return promptly from local state; they must
not wait for a phone, socket, firmware service or an unbounded lock. Future I/O
may update a synchronized bounded cache outside the render thread.

The manager samples providers, applies validity/freshness/selection policy and
hands one immutable selected snapshot to the existing widgets/layouts. Keep that
snapshot for the whole frame to avoid mixed-source updates during preparation.
One provider failure must not discard another provider's independent valid data.
The existing whole-read failure behavior in provider.c applies per provider;
future resolution combines only successful eligible fields.

## Signal contract

Use the existing temperature, range, media and navigation (trip/consumption
removed in P2-07)
semantics/units described in [module.md](module.md). A provider may supply any
verified subset; omitted signals are MISSING, never zero-by-default live values.
Capability information should distinguish unsupported fields from temporary
unavailability when the manager needs that distinction. No automatic conversion
of arbitrary phone fields into vaguely similar vehicle signals.

For example, a phone's battery range must not become the diesel driving-range
widget; next-turn distance must not become final-destination remaining distance.
Trip identity/reset and consumption averaging intervals must match before an
alternate provider can populate those widgets. No such Android signals are
assumed available today.

Retain availability, provenance and sample age per datum. Before live/mixed-source
use, add minimal provider-origin/session metadata alongside validity:

- Provider origin: vehicle, Android or mock, separate from MOCK/VERIFIED quality.
- Session/generation (and sequence for future received samples) to invalidate
  disconnected/replaced sessions and reject stale/out-of-order updates.
- A clear signal freshness policy. The current 30s host threshold is a prototype
  policy, not a measured vehicle/phone update rate; future per-signal policies
  need evidence before replacing it.

These additions are not present in the current structs. Extend the internal model
deliberately when the corresponding manager/adapter is implemented; preserve
existing signal semantics and formatting rather than changing widget logic to
parse transport data. The in-process C structs are not a stable wire protocol.
Any later wire schema needs explicit version/capability handling and decoding.

## VehicleDataProvider — Phase 2

Use already demonstrated read-only data paths where verified, not new vehicle
control commands or native hooks. [VehicleDataModule](../../java_patch/com/luka/carplay/core/VehicleDataModule.java)
demonstrates temperature/range DSI readings and unit conversion. In the original
baseline it ran for CarPlay only; P2-08 adds config-gated Android Auto Kombi-only
binding. See
[CarPlayApp](../../java_patch/com/luka/carplay/core/CarPlayApp.java). Its existing
bus message goes toward the CarPlay hook, not a dashboard subscription API.

Reusing those values for an Android Auto dashboard requires proving safe lifecycle,
read-only publication and actual signal availability on this vehicle. That is
plausible but unverified; do not change subscriptions/session ownership merely
to claim this provider exists. Navigation/media use existing system sources only
when their semantics and freshness are verified. Trip/consumption sources remain
unknown in the investigated active modules.

On the host, continue using the separate MockDataProvider. If a real adapter
cannot establish a signal, VehicleDataProvider must report MISSING/INVALID;
it must never silently synthesize a vehicle reading. This design honors the
Phase 2 adapter role without inventing compatibility or installing anything now.

## AndroidDataProvider — Phases 4–5

Phase 3's phone app is initially standalone. Phase 4 chooses/establishes the
connection and validates received data; AndroidDataProvider exposes a normalized
local snapshot from that data. Phase 5 selects those fields for existing widgets.

Do not couple the provider interface to Android Auto or AAWireless. Transport,
platform APIs and available additional signals remain unknown. No Android app,
socket/Bluetooth/USB code, receiver stub, background listener, transport library
or protocol implementation is added during Phase 2.

Clock domains need explicit treatment: phone monotonic time cannot be compared
directly with MMI monotonic time. A future decoder must establish a conservative
age mapping to the manager clock or mark freshness unknown. Receipt time alone
must not make an old phone measurement fresh. Session/sequence validation,
producer data age and bounded clock uncertainty belong in the future adapter's
contract before any live signal is accepted.

Navigation remains an atomic group. Preserve the oldest contributing sample
time and route/session identity so a fresh destination name cannot revive old
distance/duration. Disconnect invalidates the Android cache; reconnect starts a
new generation and cannot reuse old session data as fresh.

## Combining providers without rewriting widgets

For one source, the manager can consume the existing callback directly. When
multiple providers actually exist, use a small resolver/composite wrapper that
also satisfies `db_provider`; avoid a framework or plugin registry in advance.

Selection must be explicit per signal/capability and documented in configuration:

| Signal family | Intended selection rule (design, not implemented) |
| --- | --- |
| Verified vehicle measurements | VehicleDataProvider is authoritative by default. Android must not silently override it. |
| Additional Android information mapped to an existing semantic field | Use Android only where semantics/units are verified and that source is explicitly selected. |
| Media/navigation supplied by multiple sources | Select an explicit source/session as a consistent group; do not mix destinations or alternate by arrival order. |
| Unsupported/stale/invalid selected field | Display the existing placeholder/status. Only use an explicitly configured, semantically equivalent and fresh fallback. |
| Host mock examples | Explicit preview/development mode only, with MOCK provenance. Never an automatic fallback for failed live data. |

Do not implement last-writer-wins or concatenate data from incompatible trips.
Preserve selected origin, quality and timestamp for diagnostics/UI. A field may
switch source only under the configured policy; one Android disconnect must not
erase independent vehicle values. Failure of all eligible providers yields
unavailable fields or manager suppression under its documented safety policy.

Adding AndroidDataProvider should require the new adapter/cache, source metadata
and resolver/config wiring. The common manager path, semantic widget formatters
and Compact/Extended layout contracts remain shared. A new data concept can add
an optional signal/widget later; it does not justify a parallel Android dashboard.

## Work boundaries and later acceptance

P2-03 should keep manager inputs behind `db_provider`, not hard-code db_mock_read
into manager/widgets. The preview CLI may remain explicitly mock-only. Do not
implement transport, multiple live providers or a resolver merely to complete
the Phase 2 host configuration step.

At the final testing stage, when explicitly requested, verify provider-independent
widget behavior, mixed source selection, partial provider failure, stale/unknown
age, reset/disconnect/session generation, capacity and units. Until then document
these acceptance targets without executing tests. P2-04 now runs the host
checks that exist for the single-provider path. All host-only/vehicle safety boundaries remain in force.
