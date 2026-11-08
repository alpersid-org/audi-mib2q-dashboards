# Phase 2 Readiness – Custom Compact and Extended Dashboards

Companion to `PHASE2_HANDOVER.md`. Labels: **[CONFIRMED]** · **[INFERRED]** · **[UNKNOWN]**.

## 1. Verdict

**Ready to start the research and prototyping phase. Not ready to implement production dashboards.**

- **What's in place:** the Phase 1 stack gives a proven path to put HU-rendered pixels into the Virtual Cockpit's map area: managed displayables 98/99, custom cluster contexts 80–82, and a Java single-writer context switch.
- **What's open:** whether a dashboard can go beyond the map area, how it is selected by the driver, and which vehicle signals are available are all open questions. They must be validated on the car first.

## 2. Confirmed capabilities

| Capability | Evidence |
|---|---|
| HU-rendered video shown full-size (1440×540 surface) in the VC map area, wide and Classic layouts | Vehicle test (Waze, Google Maps); `altscreen_render` → displayable 99 → ctx 81/82 |
| A transparent GLES overlay composited above the map (displayable 98, ctx 80/82) | Source (`maneuver_render`, `DisplayManagerMIB2High`); maneuver arrow seen on the cockpit |
| Custom cluster contexts accepted by this unit's DisplayManager | Vehicle test (ctx 81/82 in use) |
| VC layout/stage known to the HU (FctID 54) and switching live | Source + vehicle test (VIEW button changes the phone layout) |
| Android Auto receiver matches the `P5152` profile (pinned) | SHA-256 of the unit's `gal`/`libautoreceiver.so` vs `aa_hook.c` |
| Wired and wireless (AAWireless Gen1) Android Auto with the cluster map | Vehicle test |
| Turn, distance, street and ETA data reaching the VC via BAP | Vehicle test |
| Raw steering-wheel key observation (key 40) and VC-tab gating | Source + vehicle test (short/long press) |
| OEM functions intact (radio, BT phone, Audi navigation, PDC/camera, wheel controls) | Vehicle test |
| Full reversibility (GEM uninstall, `.carplay-stock` configs, M.I.B. backup on the laptop) | Source + files present |
| Stock HMI image (`lsd.jxe`) available offline for class/DSI surveys | Local backup |
| Reproducible build path (Docker QNX 6.5 ARMv7 + JDK 8) | Upstream scripts |

## 3. Technical limitations (known)

| Limitation | Status |
|---|---|
| The VC draws its own dials, needles and fixed widgets; the HU only supplies the map/KDK video area | **INFERRED** (strong, from upstream RE of AU491 and the compositing chain) |
| Displayable 103 cannot be created; only 98/99 proven as custom ids | **CONFIRMED** (code comment) / others **UNKNOWN** |
| 30 fps maximum into the cockpit (encoder capture every 33 ms) | **CONFIRMED** (upstream measurement/docs) |
| One Venus video core; concurrent decoder state changes can hang it and trigger a watchdog reboot | **CONFIRMED** (upstream issue #14, mitigated by the yield protocol) |
| No Android Auto cluster zoom; theme fixed per session; Android Auto layout size/density only at connect | **CONFIRMED** |
| Jar changes load only after a cold boot; killing j9 restarts the system | **CONFIRMED** |
| HMI heap `-Xmx60m`; `/mnt/app` about 162 MB free; `/mnt/system` about 3.5 MB free | **CONFIRMED** |
| Briefly cycling the ignition doesn't reboot the MMI | **CONFIRMED** (logs + vehicle) |
| No `scp`/`sha256sum`/`tar` on the unit; deployment through M.I.B. Run Custom Script | **CONFIRMED** (upstream docs; M.I.B. path used in Phase 1) |
| Must use `org.dsi.*` interfaces only (stock adapter subclass broke navigation start) | **CONFIRMED** (upstream car finding) |

## 4. Open questions

1. Can anything be drawn outside the VC map area (e.g. full-width over the dial region) in any VC mode? **[UNKNOWN]**
2. Which displayable ids and context ids can be added safely? (`dmdt gd`/`gs` read-out; `libdm_modMain.so` RE) **[UNKNOWN]**
3. How should the driver select a dashboard? (VC tab, MFW long press, GEM/MMI setting, automatic when no route) Which stock events are free? **[UNKNOWN]**
4. Which DSI signals exist and update fast enough on this A4 (RPM, temperatures, boost, gear, accelerations, TPMS, consumption)? **[UNKNOWN]**
5. Can a dashboard coexist with the Android Auto video in the same session (switch 99 ownership, or overlay on 98)? **[UNKNOWN]**
6. GPU/CPU/memory headroom with Android Auto video, the overlay and a dashboard renderer at once? **[UNKNOWN]**
7. What does EXLAP on :25010 expose, and is it usable locally? **[UNKNOWN]**
8. What exactly is the visible pixel rectangle of the map area in Classic and wide on AU491? (calibration pattern) **[INFERRED]** from upstream values, not measured
9. Behaviour with VC popups (PDC, calls, warnings), Night Vision holes and VC menu changes while a custom context is active. **[UNKNOWN]**
10. Is the Sport skin present on this car? **[UNKNOWN]**
11. Was the SVM fix logged as successful? **[INFERRED]** yes (auto-reboot); confirm from the M.I.B. log on the card.

## 5. Risks

| Risk | Impact | Mitigation |
|---|---|---|
| Wrong context/plane handling leaves the cockpit blank or hides Audi navigation | High (driver information) | Keep `ScreenModule` as the only writer; always converge to stock ctx 74 on any error; heartbeat-gated contexts; never test while driving |
| Video-core contention (extra decoder or encoder) → MMI freeze/reboot | High | No extra video-core users; follow the `hw_yield`/`hw_active` protocol |
| j9 heap exhaustion from Java-side rendering or data | High (HMI restart) | Render natively; keep Java to data and control; measure heap |
| Filling `/mnt/system` or `/mnt/app` | High (boot/config failure) | Binaries in `/mnt/app/root`; size budget; never write large files to `/mnt/system` |
| Breaking the Phase 1 Android Auto cluster map | Medium | Separate feature switches/markers; regression checklist (§6 step 8) |
| Stock class overrides diverging from the firmware | Medium | Build/link audit against `MU1329-lsd.jxe`; patch the minimum number of stock classes |
| Leaking identity data in logs or shared files | Medium | Redact the VIN/IMSI j9 line, FAZIT, serials, coding and MACs before sharing |
| GPL / proprietary class redistribution | Legal | Decide the licensing model before publishing anything |

## 6. Required validation (ordered; nothing here modifies the unit unless stated)

1. **Capture a post-reboot log set.** GEM *Save logs to SD* → route in Google Maps → *Save logs* again. Confirm `init enabled profile=P5152 gal=pinned`, `cluster.register ok=1`, `cluster.frame`, `ctx 81/82` in `carplay_java.log`, and renderer status. (Writes only to the SD card.)
2. **Measure the visible area.** Read-only: photograph the cockpit with the CarPlay-RGI *Calibration grid* (CarPlay only) or an Android Auto inset experiment, for both Classic and wide.
3. **Offline survey of `MU1329-lsd.jxe`** (decompile on the laptop): list `org.dsi.ifc.*` interfaces and the vehicle signals they carry; the DisplayManager/Kombi control APIs; VC tab/context IDs; free key events.
4. **DisplayManager capability study:** decompile `libdm_modMain.so` from a firmware image, or get a read-only `dmdt gs`/`dmdt gd` dump if a shell is available. Find the registered displayable ids and context limits.
5. **Resource baseline:** `pidin`/`hogs` and heap figures during an Android Auto cluster session (logging mod).
6. **Proof of concept, behind a marker file and with a stock fallback:** a static test pattern renderer on displayable 99 in the existing ctx 81, with no phone session. Verify placement in Classic and wide, popups, and fallback when the renderer is killed.
7. **Data prototype:** a Java module reading 3–5 confirmed DSI signals, logging rates and validity (no rendering).
8. **Regression checklist** after every unit change: Android Auto wired and wireless map, Waze and Google Maps, VIEW layouts, roller short/long, radio, BT call, Audi navigation, PDC/camera, a cold boot verified by the Audi logo, and the GEM uninstall path still present.

## 7. Prerequisites checklist for the Phase 2 engineer

- [ ] Docker images built: `qnx65-armv7-toolchain` (luka-dev), `eclipse-temurin:8`
- [ ] Stock jar/JDK placed per `upstream:stock/README.md` (from `MU1329-lsd.jxe`, kept local and never published)
- [ ] Upstream repo checked out at `b127988a68c8d690841f4a75d35717ffc01e668e`
- [ ] Laptop backup (`backups/MIB-backup-2026-11-08`) verified against `MIB-backup-2026-11-08.sha256.txt`
- [ ] SD card with M.I.B. V3.8.0 + mib2q-carplay 1.4.1 kept as the known-good rollback card
- [ ] Battery support available for every in-car session; car parked
