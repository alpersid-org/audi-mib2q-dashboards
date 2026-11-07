# Phase 2 investigation and implementation boundary

Investigated upstream `master`, commit
`b127988a68c8d690841f4a75d35717ffc01e668e` (1.4.1), on 2026-11-07.
The workspace was empty; upstream was cloned for investigation. Work now lives
in the separate `audi-mib2q-dashboards` project on `feature/custom-dashboards`,
which inherits the full upstream source and Git history. Inherited implementation
files remain unchanged. No vehicle connection or installation was performed.
This investigation preceded implementation. P2-01 now supplies the isolated
host provider/widget module and P2-02 now adds standalone host SVG layouts.
Vehicle integration remains deferred; final graphics validation is pending.
Source evidence below establishes what this checkout does, not what this
particular A4 supports.

## Compatibility facts

**Confirmed by existing code/documentation:** [README](../../README.md) reports
development on an A5 F5, MU1329/P5152, and other users' A4/P5092 results. It lists
AAWireless **TWO+**, including firmware qualifications. The AA map hook has
receiver/`gal` profiles and object-layout assumptions in
[aa_hook.c](../../aa_hook/aa_hook.c), described in
[firmware-porting](../../docs/android-auto/firmware-porting.md).

**Unsupported or unknown:** this A4's exact firmware, HMI ABI, receiver profile,
cluster geometry, original AAWireless compatibility, Galaxy S23 Ultra combination,
and Phase 1 success. A model/year match, build success, or a TWO+ report establishes
none of these. Do not enable ABI trial mode or infer offsets for this iteration.

## Architecture and controllable surfaces

| Conclusion | Evidence and confidence |
| --- | --- |
| Native C hooks run in `dio_manager` (CarPlay) and `gal` (AA); patched Java runs in the HMI. A supervisor starts renderers. | **Confirmed by existing code:** [startup/monitor](../../deploy/smartphone_integrator/), [CarPlayApp](../../java_patch/com/luka/carplay/core/CarPlayApp.java), [architecture](../../docs/architecture.md). Java maintains session ownership and module lifecycle. |
| Maneuvers use C EGL/GLES2 with a C++11 scene engine. The managed QNX window is displayable 98, RGBA, transparent when idle, 328x181 including one ECC row. | **Confirmed by existing code:** [platform_qnx.c](../../maneuver_render/platform_qnx.c), [protocol.h](../../maneuver_render/protocol.h), [render.c](../../maneuver_render/render.c), [scene/](../../maneuver_render/scene/), [cluster_surface](../../common/cluster_surface.c). |
| Shared infrastructure creates/manages a Screen window; it does not establish a safe VC region. Java selects composition. | **Confirmed by existing code:** [cluster_surface.h](../../common/cluster_surface.h), [DisplayManagerMIB2High](../../java_patch/de/audi/tghu/fwhmi/DisplayManagerMIB2High.java). |
| Phone map video is decoded using OMX hardware or FFmpeg software and presented through displayable 99. CarPlay uses tee port 19820; AA uses 19821. | **Confirmed by existing code:** [altscreen main](../../altscreen_render/main.c), [decode](../../altscreen_render/decode.c), [hardware decode](../../altscreen_render/hw_decode.c), [GLES presenter](../../altscreen_render/video_gles.c), [AA tee](../../aa_hook/aa_tee.c). |
| Context 80 composes `{98,101,102,33}`; 81 is `{99}`; 82 is `{98,101,102,99}`. 33 is the stock map; 101/102 are stock KDK backings. G24 uses a different path. | **Confirmed by existing code:** `defineContexts()` in [DisplayManagerMIB2High](../../java_patch/de/audi/tghu/fwhmi/DisplayManagerMIB2High.java). Some older doc/comments omit 81/82; the actual context table is authoritative. |
| AA video and the existing maneuver graphics can coexist. | **Confirmed by existing code:** context 82 and [ScreenModule.republish](../../java_patch/com/luka/carplay/core/ScreenModule.java), with [AltScreenModule](../../java_patch/com/luka/carplay/core/AltScreenModule.java) liveness fallback. This does not prove an additional dashboard can coexist safely. |
| KDK placement follows stock Layout constants, FctID 44 visibility and FctID 54 stage. Known source crops include `(59,27,210,153)` and `(0,0,328,180)`. | **Confirmed by existing code:** [ClusterLayerController](../../java_patch/com/luka/carplay/cluster/ClusterLayerController.java), [CombiMapController](../../java_patch/de/esolutions/hmi/widgets/audi/evo/high/widgets/CombiMapController.java), [viewport helper](../../maneuver_render/visible_area.h). See [geometry](../../docs/cluster/kdk-geometry.md). These are guidance crops, not dashboard safe areas. |
| A 2D overlay renderer could be adapted for dashboard primitives. | **Technically plausible but unverified:** [render.h](../../maneuver_render/render.h) exposes clipped overlay meshes; [lane_panel.cpp](../../maneuver_render/scene/lane_panel.cpp) uses them. There is no general dashboard text renderer. Glyph shaping, GL state, cost, ownership and visibility need validation. |
| A free displayable ID or video SafeArea guarantees a safe dashboard slot. | **Unsupported or unknown:** no dashboard context, ownership policy or non-critical reserved rectangle is demonstrated. SafeArea in [AA UI config](../../aa_hook/aa_uiconfig.c) and AltScreen config arranges phone content; it is not an instrumentation protection boundary. |
| Arbitrary VC widgets, speed/warning layers, or a standalone VC application can be controlled. | **Unsupported or unknown:** the demonstrated surface is HU graphics carried through the existing cluster video/navigation pipeline, not arbitrary ownership of the instrument cluster. |

## Data inventory

| Signal | What is demonstrated | MVP decision |
| --- | --- | --- |
| Outside temperature | **Confirmed by existing code:** [VehicleDataModule](../../java_patch/com/luka/carplay/core/VehicleDataModule.java) reads `DSICarKombi.updateBCOutsideTemperature`, checks attribute validity, converts C/F to 0.1 C. It deliberately does not trust the nested `state` field on the upstream development car. | Mock; availability on this car is **unknown**. |
| Driving range | **Confirmed by existing code:** same module uses `BCTotalCurrentRange`, falls back to `BCCurrentRange1`, checks value state, converts miles/km to km. [Vehicle hook](../../hook/vehicle/vehicle_hook.h) defines `CMD_VEHICLE` validity bits. | Mock; total/fallback meaning must be checked on this diesel. |
| Vehicle source during AA | **Confirmed by existing code:** `CarPlayApp.MODULE_OWNERS` starts `VehicleDataModule` for CarPlay only. The 14-byte binary snapshot is a command toward the CarPlay hook, not a dashboard subscription API. | No bus interception or lifecycle change. A future read-only HMI snapshot adapter is **plausible but unverified**. |
| Navigation distance/time/destination | **Confirmed by existing code:** [AaBridge](../../java_patch/com/luka/carplay/aa/AaBridge.java), [AaRgState](../../java_patch/com/luka/carplay/aa/AaRgState.java), [AaTripFeed](../../java_patch/com/luka/carplay/aa/AaTripFeed.java), [aa_trip](../../aa_hook/aa_trip.c); next-turn distance differs from final-destination distance. Rich metadata is app dependent, with independent sample ages and UTC/system time handling. CarPlay uses [RGD](../../hook/routeguidance/rgd_tlv.c). | Mock final-destination distance and duration, not inferred ETA. Rich AA trip details have host tests; upstream docs say car validation pending. |
| Current media | **Confirmed by existing code:** `AndroidAuto2ListenerDistributor.updateNowPlayingData(TrackData, validity)` forwards existing DSI events; [AaCoverArt](../../java_patch/com/luka/carplay/aa/AaCoverArt.java) and [CoverArt](../../java_patch/com/luka/carplay/coverart/CoverArt.java) handle artwork. | Mock title/artist. Exact TrackData fields, validity, freshness and session ownership for a dashboard adapter are **unverified** here. Artwork alone does not establish title/artist availability. |
| Trip distance / average fuel consumption | **Unsupported or unknown:** no active producer with verified units, validity and trip/reset semantics was found in the investigated modules. Imports or stock listener method signatures alone are insufficient. | Mock; no new DSI/CAN query. No DPF work. |
| System state | **Confirmed by existing code:** renderer heartbeats/readiness, video liveness, phone session owner, viewport/stage, route presentation state exist in [RendererServer](../../java_patch/com/luka/carplay/rgd/RendererServer.java), ScreenModule and AltScreenModule. | Future suppression inputs only; no production wiring in this MVP. |

## Green Engineering Menu and input

**Confirmed by existing code:** [build_gem_menu.sh](../../scripts/build_gem_menu.sh)
generates `.esd` script entries and wrappers. [run.sh](../../deploy/gem_menu/run.sh)
dispatches actions, often writing persistent marker/config files. The
[menu installer](../../mods/menu_install_MoreIncredibleBash/mod/custom.sh) copies
engdefs files and registers the menu; existing menu tests run against host fixtures.
The existing [CarPlaySettings](../../java_patch/com/luka/carplay/connectivity/CarPlaySettings.java)
also persists settings for MMI options. Physical input exists, but no dashboard
input contract is demonstrated.

**Technically plausible but unverified:** a future GEM page could select enable,
compact, extended and restore-default using a single validated config file and
existing menu generation. It requires a working, safely gated dashboard consumer
first. **Unsupported or unknown:** a safe dashboard activation path in this car.
No GEM buttons or vehicle markers are added now: they would advertise a function
which has no validated vehicle backend. No controller or steering-wheel key is
intercepted or repurposed.

## Smallest feasible increment (P2-01 data/widgets and P2-02 host SVG implemented)

Safe VC rendering cannot be established from this checkout alone. Use the
explicitly permitted standalone renderer fallback. Implement an isolated host C99
module with fixed-size frame commands, two layouts and a mock provider. Reuse
`cr_rect_t`/`cr_rect_equal` from `maneuver_render/visible_area.h`; do not use its
invalid-input fallback, which chooses a guidance crop. Reject undersized surfaces
instead. No runtime changes to existing renderers, hooks, Java, install payloads,
supervisors or menu scripts. No QNX build/install target.

| Component | MVP boundary |
| --- | --- |
| Dashboard Manager | Default disabled; creates a complete frame or an empty suppressed frame. Clears on invalid config, provider failure or unsupported viewport. |
| Renderer adapter | Host SVG export of rectangle/text commands with explicit clipping. No Screen window, GL context, sockets or live renderer protocol. |
| Widget system | Small typed formatter switch for temperature, range, trip, consumption, media and navigation; missing/stale/invalid data stays explicit. |
| Data provider | Callback fills an owned snapshot, canonical units, per-field sample time and provenance. Independent of DSI/phone transport. Only a deterministic mock implementation in Phase 2. |
| Configuration Manager | Strict bounded versioned text format; absent/invalid config disables. Selecting layout does not implicitly enable. Restore default returns disabled compact. |
| Layout Manager | Proposed Compact 2x2 and Extended 3x2 grids, in source pixels. Host wide 800x252 and tall 640x360 are saved host preview dimensions, not vehicle layouts. Reject 210x153 and 328x180 maneuver crops in the initial prototype as a deliberate design limit; readability still requires host validation. |

No additional framework, dependency, rendering loop or animation is needed. SVG
text uses a host font and is not proof of QNX font support. A future provider can
fill snapshots without changing widgets; Android companion applications and new
Android-to-MMI communication remain entirely outside this iteration.

Tests will cover strict config/fallback, units/age/provenance, provider failure,
frame clearing, clipping and layout bounds, deterministic SVG, hostile text, and a
bounded host preparation benchmark. Run existing host scripts and record missing
stock assets/toolchains separately. Host results cannot establish vehicle safety.
