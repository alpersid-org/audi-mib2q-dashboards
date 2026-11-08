# Phase 2 Technical Handover – Custom Virtual Cockpit Dashboards

**Vehicle:** Audi A4 B9 (8W0), model year 2017 · **Head unit:** MHI2Q (MIB2 High, Qualcomm) · **Cluster:** Virtual Cockpit `KI_FPK_AU491`
**Phase 1 outcome:** Android Auto map video on the Virtual Cockpit, wired and wireless (AAWireless Gen1), fully working and vehicle-tested.
**Prepared:** 2026-11-08 from a read-only investigation of `C:\Users\HP\Downloads\Audi - mib\` plus the upstream mib2q-carplay source at the exact installed commit.
**Audience:** an AI coding agent or engineer starting Phase 2. No Phase 2 code exists yet.

---

## 0. How to read this document

Every technical statement carries one label:

| Label | Meaning |
|---|---|
| **[CONFIRMED]** | Backed by a file, code, log or vehicle test we hold. The evidence is cited. |
| **[INFERRED]** | Technically plausible from the evidence, but not directly verified on this vehicle. |
| **[UNKNOWN]** | Needs investigation or vehicle validation before it can be relied on. |

**Evidence sources and how they're cited**

| Prefix | Location | Notes |
|---|---|---|
| `local:` | Relative to `C:\Users\HP\Downloads\Audi - mib\` | `<unit>` stands for the backup sub-folder, whose name contains a unit serial and is redacted on purpose. |
| `upstream:` | Repository `kamgurgul/mib2q-carplay` at commit `b127988a68c8d690841f4a75d35717ffc01e668e` (tag `v1.4.1`) | The release zip holds no C/Java source, so source was read from GitHub at this exact commit. |
| `vehicle test` | Photos and observations from the Phase 1 sessions on 2026-11-08 | |

**Confidence hierarchy:** vehicle test > unit logs/backup > upstream source > upstream docs. Several upstream docs are themselves labelled "verified-on-car" for a different car (an Audi A5 F5 with MU1329, train P5152). Where that is the only evidence, this document says so.

---

## 1. Hardware and firmware

### 1.1 Head unit identity [CONFIRMED]
Sources: `local:backups/MIB-backup-2026-11-08/<unit>/MU1329-allversions.txt`, `…/SWDL/Cached*.txt`, `…/SWDL/Variant.txt`.

| Item | Value |
|---|---|
| SW train | `MHI2Q_ER_AUG22_P5092` |
| MU (main unit SW) | `1329` |
| RCC version | `U9018` (RCC label `MIB2CLU10-Trunk_rcc2_C1-18092D`) |
| SW part number / HW version | `8W5035036` / `033` |
| EEPROM variant (Variant1) | `MU-P-LN-EU` |
| Metainfo2 variant (Variant2) | `FMQ-P-TNL-EU-AU-MLE` |
| Car model (from coding) | Audi A4 B9 (8W0) |
| HMI build | `HMI_AU_MIB2_HQC_ER_G22_C10_180901` (`APP: DEV_MMX2QC_AU_ER_G22_201PROD`, changelist 10757131) |
| Framework | MMX 5.51.6 / RCC 5.51.0 (`MIB2MAIN I202 CI70`) |
| Java | IBM J9, DSI `2_69`; HMI image `/ifs/lsd.jxe` |
| Toolchain / OS | `RL_QNX_OS_650SP1_QC_MMX2_C10ES00.10A`; `uname`: `QNX mmx 6.5.0 … APQ8064 … armle` |
| Navigation | `CLU10_17123MIB2MAIN_QC_B18.07.2.100345`; NavDB `P187_N60S5MIBH3_EU_NT_Q3.19` |
| Depot changelist `mib2_tools` | `10757182` (the same changelist upstream cites for the P5152 `gal` build; see §2.3) |

Deliberately omitted from all handover documents: VIN, IMSI, FAZIT/unit serial, MMX board serial and the long-coding string. They exist in the local backup and logs only.

### 1.2 SoC, memory, storage [CONFIRMED]
Sources: `local:backups/carplay_logs-2026-11-08/002/pidin_info.txt`, `…/df.txt`, `…/<unit>/MU1329-Partition.txt`.

- **CPU and RAM:** 4× Qualcomm Krait at 1512 MHz (APQ8064), 2048 MB RAM, about 898 MB free at runtime.
- **`/mnt/app`** (qnx6, mounted read-only; `mount -uw` needed to write): about 995 MB, **84 % used, about 162 MB free**. Holds `/mnt/app/root/hooks`, `/mnt/app/eso/hmi/…` and the GEM files.
- **`/mnt/system`** (qnx6, read-only): about 8 MB, **57 % used, about 3.5 MB free**. Holds the `/etc/eso/production/*.json` configs the mod patches.
- **`/tmp`** is `/dev/shmem`: RAM-backed, **cannot hold directories**, has no truncate-safe rename, and is cleared on reboot. Sources: `upstream:docs/deploy/install.md` §Traps and `upstream:java_patch/com/luka/carplay/aa/AaClusterView.java` header.
- **HMI JVM limits:** `-Xmx60m -Xmo50m -Xmn10m`. Source: `local:…/002/pidin_ar.txt`, j9 command line. Any Java-side dashboard code shares this heap with the whole HMI.

### 1.3 System architecture (two QNX nodes) [CONFIRMED unless marked]
- **MMX** (application processor, QNX 6.5). It runs:
  - the HMI JVM (`j9 … -jxe:/ifs/lsd.jxe`);
  - `displaymanager 2_lvds`;
  - `videoencoderservice`;
  - `smartphone_integrator`, and on demand `gal` (Android Auto) / `dio_manager` (CarPlay);
  - `navStartup` and `PNavApp`;
  - `screen`, plus `devp-iso-mmx-mib2` ×4 (MOST/MLB isochronous RX/TX).

  Source: `local:…/002/pidin_ar.txt`.
- **RCC:** a second QNX node reachable as `/net/rcc/…`, with flash file systems `/mnt/efs-persist` and `/mnt/efs-system`. M.I.B. writes there (SWDL flags, FEC containers, EEPROM via `svm`). Source: `local:…/<unit>/MU1329-Partition.txt` and `local:downloads/MIB-3.8.0/extracted/…/apps/launcher`.
- **IOC version:** 9267 (`allversions.txt`).
- **Cluster link:** the head unit renders and H.264-encodes the cluster's map area and sends it over MOST to the Virtual Cockpit, which decodes it and overlays its own graphics and BAP text.
  - Process presence (`videoencoderservice`, MLB iso TX drivers) [CONFIRMED]: `pidin_ar.txt`.
  - Pipeline details [CONFIRMED in upstream reverse-engineering docs, not re-derived here]: `upstream:docs/cluster/compositing.md`, `upstream:docs/re/firmware/komo-widget-video.md`.

### 1.4 Virtual Cockpit [CONFIRMED / INFERRED]
- **Cluster identity [CONFIRMED]:** `KI_FPK_AU491`, App 113/137, flash driver 991, INIC 0x10000. Source: `MU1329-version.txt`; also shown in the SWDL device summary during the M.I.B. install.
- **Same cluster as upstream research [CONFIRMED]:** upstream decompiled the same cluster firmware family ("VC AU491 `gtf2`"). Source: `upstream:docs/cluster/kdk-geometry.md`.
- **Layouts on this car [CONFIRMED vehicle test]:** the user's cockpit has *Classic* (big dials, small map between them) and the *wide/large map* view. The phone map displayed correctly in both.
- **Sport skin [UNKNOWN]:** whether this car also offers the Sport skin (`LayoutMIB2HighB9Sport`, small map left of a central rev counter) is not established.
- **Panel [INFERRED]:** 12.3″, 1440×540, about 125 PPI. Source: `upstream:aa_hook/aa_hook.h` (`AA_VIEW_W/H`, `AA_DPI_DEFAULT` comment). The HU cluster terminal surface used by the mod is 1440×540 and displays correctly (vehicle test). The panel's native resolution itself was not read from the car.

---

## 2. Installed software

### 2.1 M.I.B. – More Incredible Bash V3.8.0 [CONFIRMED]

**Package and verification**
- Source: GitHub tag `V3.8.0`, commit `0e041895a1ce36da4cb0835437d48aa5dbbf262a`, marked "pre-release" upstream.
- Download: `local:downloads/MIB-3.8.0/MIB-V3.8.0-source.zip`, SHA-256 `af4d8e3c97cb8ac56d373f0619e9b241b27e5daf57965443f0522f13b0e8fd40`.
- Verification: all 318 files were compared with the git tree (blob SHA-1). 315 match exactly; 3 (`metainfo2.txt`, `esd/*.esd`) differ only by CRLF, which the repo's `.gitattributes` mandates (`local:CHECKLIST.md`).
- Version label: `VERSION` reads `V3.8.0 "CarPlay Maps VC omonob (QCDWJ) Edition"`. The GEM title in `esd/Launcher-sda0.esd` line 18 still says "V3.7.2 MHIG Edition" (cosmetic upstream leftover).

**What the Launcher changed on the unit** (`apps/launcher -all`, lines 27–110, matching `<unit>/MU1329-LOG.txt` lines 88–101):
1. Created the RCC flag `/net/rcc/mnt/efs-persist/SWDL/skipMostPopup.txt`.
2. Deleted `/net/rcc/mnt/efs-persist/FEC/IllegalFecContainer.fec` and `WithdrawnFecContainer.fec`, if present.
3. Cleaned `SWDL/*.old`, `SWDL/Log/service/*` and two SWDL info files.
4. Checked GEM: `AppDevelopment.jar` was found, and the GEM update was **skipped** (unknown SHA-1, so no GEM jar replaced).
5. Created the symlink `/mnt/app/eso/hmi/engdefs/z_Launcher-sda0.esd → /net/mmx/fs/sda0/esd/Launcher-sda0.esd`. **M.I.B. menus therefore exist only while the card is in SD1.**
6. Enabled GEM via persistence: `udp -key 3221356557 -ns 0 -type b -value 1`, then commit key 1 ("Development Mode / block 5F adaptation").

**Other M.I.B. actions**
- **Backups:** M.I.B. took a basic backup during install and a standard backup later (`MU1329-LOG.txt` lines 57–146). It also kept "backup part 1" on the unit in `/net/mmx/mnt/boardbook/MU1329`.
- **SVM fix:** `esd/scripts/svm.sh` → `apps/svm -f`. It writes `SvmInfo.bin`/`CfgAck.bin` and an EEPROM value at address `0x3f0`, then reboots on success (`apps/svm` lines 73–107).
  - The unit restarted by itself. [INFERRED: success, because the reboot happens only on success.]
  - The SVM log line itself was **not** re-read after the fix. [UNKNOWN]
- **Unused M.I.B. features:** the "CarPlay Maps VC" (omonob) content in `mod/carplay_maps_vc` and `apps/CarPlayMapsVC.sh` was **not** run. [CONFIRMED: only Backup, SVM fix and Run Custom Script were used.]

### 2.2 mib2q-carplay 1.4.1 [CONFIRMED]

**Package and verification**
- Release: `local:downloads/mib2q-carplay-1.4.1/mib2q-carplay-1.4.1.zip`, SHA-256 `4fd2926e8300dd2f303e9b2264fa7f06991c77405eae1bd4847526748be03fdb`, matching the GitHub release asset digest.
- All 92 files match the package's internal `SHA256SUMS.txt`.

**Install run**
- Installed with M.I.B. → `advanced_settings` → `run_custom_script` → `/mod/custom.sh`. The in-car screen output ended "DONE. mib2q-carplay 1.4.1 installed." with no WARN/FAILED lines.
- [INFERRED] The installer likely ran twice. `dio_manager.json.carplay-stock` exists (`local:…/002/carplay_stock_ls.txt`), yet the observed run printed "dio_manager.json IDs already registered". The installer only creates that backup when it changes the file (`SD-CARD/mod/install/custom.sh` lines 240–242). Harmless: both stock backups exist.

**Files installed on the unit** [CONFIRMED: install console output plus `local:…/002/hooks_ls.txt`; install logic in `SD-CARD/mod/install/custom.sh` `flat_dest()` lines 41–51]

| On-unit path | Purpose |
|---|---|
| `/mnt/app/root/hooks/aa_startup.sh` | `children.gal` wrapper: starts the renderer monitor, then `exec`s stock `gal` with `LD_PRELOAD=libaa_cluster_hook.so` |
| `/mnt/app/root/hooks/libaa_cluster_hook.so` | Android Auto cluster-display hook (preloaded into `gal` only) |
| `/mnt/app/root/hooks/altscreen_render` | H.264 decoder/presenter for displayable **99** (CarPlay tee :19820, Android Auto tee :19821) |
| `/mnt/app/root/hooks/maneuver_render` + `flag_atlas.rgba` | GLES maneuver-arrow overlay on displayable **98** (TCP :19800 from Java) |
| `/mnt/app/root/hooks/libcarplay_hook.so`, `libaltscreen111_mhi2q.so` | CarPlay iAP2 and AltScreen hooks (preloaded into `dio_manager`, CarPlay only) |
| `/mnt/app/root/hooks/carplay_startup.sh`, `carplay_monitor.sh`, `carplay_processes.sh`, `carplay_cleanup.sh` | CarPlay child wrapper; the monitor is shared by both phone types |
| `/mnt/app/root/hooks/nav_boost` | Power-on navigation start-up priority helper |
| `/mnt/app/eso/hmi/lsd/jars/carplay_hook.jar` | Java 1.4 class overrides on j9's boot classpath (HMI side of everything) |
| `/mnt/app/eso/hmi/engdefs/mib2q-carplay.esd` + 30 scripts under `…/engdefs/scripts/carplay/` | GEM page "CarPlay-RGI" |

**Configs patched in place, each backed up once as `<file>.carplay-stock`** [CONFIRMED: `custom.sh` lines 58–59 and 117–243; `carplay_stock_ls.txt`]
- `/mnt/system/etc/eso/production/smartphone_integrator.json`: the `carplay` and `gal` children are replaced. The `gal` child now has `exec: aa_startup.sh`, `path: /mnt/app/root/hooks`, with the stock timeouts preserved (`local:…/002/config/smartphone_integrator.json`; the stock child is in `upstream:deploy/smartphone_integrator/aa_child_stock.json`).
- `/mnt/system/etc/eso/production/dio_manager.json`: iAP2 message IDs added (CarPlay only) and the NMEA bit 0x08 (PASCD).
- Removed if present, with no backup: M.I.B.'s `NavActiveIgnore.jar` / `navignore_*.jar`. They were not present here, because that M.I.B. option was never used.

### 2.3 Android Auto receiver compatibility [CONFIRMED]

The hook writes into objects inside the stock `libautoreceiver.so`, so it only runs on a receiver it has a profile for (`upstream:aa_hook/aa_hook.c`, `profiles[]` lines 83–98; `match_profile()` lines 565–578).

| Unit file (copied by the logging mod to `local:backups/carplay_logs-2026-11-08/aa/`) | SHA-256 | Match |
|---|---|---|
| `libautoreceiver.so` (1 069 943 B) | `6ef4abea8264ab7787b0ecb3fe278a65488301f703148d0175010f7b228bdaf6` | byte-identical to the MU0918 receiver all profiles share |
| `gal` (1 405 891 B) | `a659a9551b139910937539e64d28ac1af95617461e886f3882b7bd2897acc0cf` | exactly the `gal` of profile **`P5152`**, so it runs **pinned**, not trial/unpinned |

So the P5092 unit runs the hook under the `P5152` profile, the same build upstream validated on an A5 F5. The working cockpit map is the vehicle confirmation.

---

## 3. Technical architecture (Android Auto → MMI → Virtual Cockpit)

### 3.1 Process topology [CONFIRMED in source; runtime confirmed by vehicle test]
Sources: `upstream:docs/android-auto/overview.md`, `SD-CARD/carplay/aa_startup.sh`, `upstream:docs/architecture.md`.

```text
smartphone_integrator (boot-resident)
  └─ on Android phone connect: children.gal → /mnt/app/root/hooks/aa_startup.sh
        ├─ writes /tmp/aa_wrapper.log, owner file /tmp/aa_supervisor.owner
        ├─ starts carplay_monitor.sh (LD_PRELOAD cleared) → adopts/starts
        │     ├─ maneuver_render   (displayable 98, TCP 127.0.0.1:19800 from Java)
        │     └─ altscreen_render  (displayable 99, reads tee :19821 for Android Auto)
        └─ exec /mnt/app/eso/bin/apps/gal (same PID) with LD_PRELOAD=libaa_cluster_hook.so
              unless /mnt/app/root/aa_cluster.off exists

HMI JVM (j9, lsd.jxe + carplay_hook.jar on -Xbootclasspath, alive from boot)
  ├─ AndroidAuto2ListenerDistributor (patched stock) → AaBridge → CarPlayApp (owner = Android Auto)
  ├─ ScreenModule  – the ONLY cluster context writer (switchContext / setUpdateRate)
  ├─ AltScreenModule – watches /tmp/altscreen_render.live → selects ctx 81/82
  ├─ ClusterLayerController – positions/opacities planes 98/99/101/102, publishes /tmp/aa_cluster_view
  └─ RouteGuidance/BAPBridge – single BAP writer to the cluster (arrow, distance, route text, ETA)
```

### 3.2 The Android Auto cluster-display hook (`libaa_cluster_hook.so`) [CONFIRMED source]
Source: `upstream:aa_hook/*.c,h`.

1. **Second video sink.** It registers an extra H.264 sink in the 2016 receiver: 1920×1080 (codec resolution 3) with margins 480×540, so the phone lays out a centred **1440×540 viewport**. It also adds an input endpoint for display 1. The private vtable means nothing reaches the stock main-screen renderer (`aa_hook.c` header lines 9–24; constants in `aa_hook.h` lines 54–63).
2. **Protocol version.** It requests Android Auto protocol 4.3 (which the phone needs for resizable cluster layouts) and rewrites the phone's answer to 1.7 for the old receiver. It also suppresses "unexpected message" replies that made phones reset the link.
3. **Safe area per cockpit view.** The VideoConfiguration field-11 `UiConfig` carries insets plus theme, built in `aa_uiconfig.c` `uiconfig_body()` lines 125–140.

   Presets (top, bottom, left, right; viewport pixels, then offset by 240/270 into the 1920×1080 frame), `aa_uiconfig.c` lines 52–56:

   | View | Top | Bottom | Left | Right |
   |---|---|---|---|---|
   | full | 97 | 167 | 350 | 370 |
   | classic | 97 | 167 | 510 | 510 |
   | sport | 78 | 146 | 484 | 394 |

   - **View source:** the view comes from Java via `/tmp/aa_cluster_view`, a fixed 8-byte record: `full   \n` / `classic\n` / `sport  \n` (`upstream:java_patch/com/luka/carplay/aa/AaClusterView.java`).
   - **View changes:** a change sends AAP message **0x8009** `UpdateUiConfigRequest`. It is checked at most every 300 ms (`aa_relayout_tick()` lines 264–302).
   - **Theme is fixed for the session:** a mid-session theme change dropped the phone session (issue #19), so the theme is set at connect only. Dark is the default.
4. **Navigation translation.** New NavigationStatus messages 0x8006/0x8007 are translated in place to the legacy 0x8004/0x8005 the receiver understands, so stock DSI turn events keep flowing (`aa_navxlate.c` header). Side outputs:
   - `/tmp/aa_lanes`: lanes of the current step;
   - `/tmp/aa_trip`: a 320-byte big-endian `AAT1` record holding final destination, distance and remaining time. 30 s expiry; checksum (`aa_trip.c`; layout in `upstream:java_patch/com/luka/carplay/aa/AaTripFeed.java`).
5. **Loopback tee.** Every cluster access unit goes to a TCP tee on `127.0.0.1:19821`. The tee keeps AUs since the last IDR (with SPS/PPS), so a late renderer starts on a keyframe. The receiver thread never blocks, and SIGPIPE is blocked (`aa_tee.c` header; issue #48).
6. **Decoder yield.** Both decoders share **one Venus video core**. Before `gal`'s main-screen decoder changes state, the hook stamps `/tmp/altscreen_render.hw_yield` and waits up to 1.5 s for `altscreen_render` to release the core (`aa_yield.c` header; issue #14: simultaneous state changes hung the core, then MMI watchdog reboot).
7. **Log:** `/tmp/aa_cluster_hook.log`. Runtime marker files live in `/mnt/app/root/` (§5.4).

### 3.3 Video rendering path to the cockpit [CONFIRMED source; end-to-end CONFIRMED by vehicle test]
1. `altscreen_render` connects to `:19821` and decodes with the Qualcomm OMX hardware decoder at 1920×1080. It blits the centred 1440×540 viewport **1:1, unscaled** into its window. Fallback: FFmpeg software decode with 2 frame threads (`upstream:altscreen_render/main.c` header lines 7–55; `upstream:docs/android-auto/overview.md` "Native").
2. The window is a **managed QNX Screen window** with `SCREEN_PROPERTY_ID_STRING="99"`, joined to the DisplayManager's manager group (`upstream:common/cluster_surface.c` lines 43–99, `cs_open_window()`). The DisplayManager adopts it as displayable 99.
3. Java selects cluster **context 81 `{99}`** (video only) or **82 `{98,101,102,99}`** (video plus maneuver overlay plus KDK backings) while `/tmp/altscreen_render.live` is fresh (`upstream:java_patch/de/audi/tghu/fwhmi/DisplayManagerMIB2High.java` lines 35–42 and 192–199).
4. The stock `videoencoderservice` captures the active cluster context (a fixed 33 ms timer, so 30 fps max), encodes it to H.264/MPEG-TS and sends it over MOST to the cluster (`upstream:docs/deploy/altscreen-mhi2q.md` §5 `ALTSCREEN111_FPS` row; `upstream:docs/cluster/compositing.md`).
5. For Android Auto, "live" lasts for the whole tee connection after the first picture. The phone sends about one picture every 6 s while the map is still, and a timeout used to make the cockpit blink (`overview.md` "Native").

### 3.4 Java HMI patches (`carplay_hook.jar`) [CONFIRMED source]
- **Language and toolchain:** Java 1.4 class overrides compiled against the stock jar, sitting on j9's `-Xbootclasspath`.
- **Load and reload:** loaded **only at JVM start**. A new jar needs a full reboot, and killing j9 is a critical-process death that restarts the whole system, about 50 s with a black screen (`upstream:docs/deploy/install.md` lines 465–468).
- **Classes relevant to Phase 2:**

| Class | Role |
|---|---|
| `de.audi.tghu.fwhmi.DisplayManagerMIB2High` | Defines cluster display contexts (§4.2) |
| `com.luka.carplay.core.ScreenModule` | Single serialized context writer. `desiredCtx = (connected && navActive) ? 80 : 74`, plus 81/82 for video. Bounces through ctx 72 for 180 ms before entering a custom context (`upstream:docs/cluster/display-contexts.md`) |
| `com.luka.carplay.core.AltScreenModule` | Heartbeat-driven video context selection |
| `com.luka.carplay.cluster.ClusterLayerController` | Plane geometry: `dm.setPosition/setCropping/setOpacity` for 98, 99, 101, 102 (lines 291–332) |
| `com.luka.carplay.aa.AaBridge`, `AaRgState`, `AaManeuverMap`, `AaLaneFeed`, `AaTripFeed`, `AaCoverArt`, `VcMapViewGate` | Android Auto DSI → shared route-guidance/BAP chain, cover art, holding back "navigation on mobile device" (BAP InfoStates 6) while the phone map is shown |
| `com.luka.carplay.core.SteeringWheelInputModule` + patched `AndroidAuto2KeyEventsController` | MFW roller press handling (§5) |
| `com.luka.carplay.core.VehicleDataModule` | DSI vehicle data reader (CarPlay sessions only; §6) |
| `com.luka.carplay.bus.CarplayBus` | TCP 127.0.0.1:19810 hook↔Java bus (CarPlay); Android Auto uses in-process `injectLocal` |

### 3.5 Navigation data path (Android Auto) [CONFIRMED source; CONFIRMED vehicle test for arrow, street text and ETA]
Phone → `gal` (0x8006/0x8007 → legacy 0x8004/0x8005) → stock DSI `DSIAndroidAuto2` next-turn events → `AaBridge` → `AaRgState` → `RouteGuidance`/`BAPBridge` → BAP FctIDs → Virtual Cockpit turn arrow, distance bar, route text and HUD. The same frames also go to `maneuver_render` (:19800) for the 3D arrow overlay.

**Vehicle test:** both Waze and Google Maps showed the arrow tile, street text and ETA line on the cockpit.

---

## 4. Virtual Cockpit rendering

### 4.1 Ownership model [CONFIRMED from upstream reverse engineering, consistent with vehicle behaviour]
- **What the VC draws itself:** dials/gauges, tabs, the lower status bar and BAP-fed text (route text, ETA, media, phone).
- **What the head unit supplies:** only the **map/KDK area**, as an encoded video stream. The HU dictates the KDK position and cropping via `IDisplayManagerKombiControl.setCropping/setPosition`; the VC firmware (`gtf2`) holds no hard-coded KDK geometry (`upstream:docs/cluster/kdk-geometry.md` §"The VC obeys; the HU dictates").
- **The visibility gate:** the VC only shows the LVDS map view when `gfxAvailable` and `LVDS_Available` are both true (`upstream:docs/re/firmware/komo-widget-video.md`). The current mod rides on stock contexts that already satisfy this; it never forces `gfxAvailable` on the FPK cluster.
- **Consequence for Phase 2 [INFERRED, high confidence]:** a "custom dashboard" rendered on the HU can only occupy the **map area of cluster terminal 1** (wide view: the large centre area; Classic: the window between the dials). The VC's own dials, needles and fixed widgets cannot be replaced or redrawn from the HU. *Validate before promising full-screen custom gauges* (§10).

### 4.2 Cluster display contexts and displayables [CONFIRMED source]
Source: `upstream:java_patch/de/audi/tghu/fwhmi/DisplayManagerMIB2High.java` lines 18–48 and 74–201. Cluster terminal = 1 (`CLUSTER_TERMINAL = 1`); main display terminal = 0.

| ctx | Displayables (front → back) | Meaning |
|---:|---|---|
| 72 | `{33}` | stock map only (`CTX_MAP`; used as a 180 ms bounce) |
| 73 | `{20, 101, 102}` | KDK + backings, no map |
| **74** | `{20, 101, 102, 33}` | **stock resting state**: map + KDK |
| 75 | `{}` | blank |
| 76 / 77 | `{58}` / `{20,101,102,58}` | alternate map (58) |
| **80** | `{98, 101, 102, 33}` | mod: maneuver overlay over the stock native map |
| **81** | `{99}` | mod: phone video only |
| **82** | `{98, 101, 102, 99}` | mod: phone video + maneuver overlay |

- **Displayables:**
  - 33: stock native map. 58: alternate map. 20: KDK (maneuver tile). 101/102: stock "987 Image" KDK backings (328×180 / 210×153).
  - **98** = `maneuver_render`, a transparent RGBA8888 GLES window, 328×181 source frame.
  - **99** = `altscreen_render`, opaque 1440×540.
- **Displayable 103 is not creatable** (comment in `DisplayManagerMIB2High.java`: "unregistered DSI id"). Which other ids the DisplayManager accepts is **[UNKNOWN]**. 98 and 99 are proven.
- **G24 clusters:** the custom contexts are disabled on G24 (`KOMBI_TYPE_G24`). This car is not G24, since the mod works.

### 4.3 Geometry, resolution and frame rate
- **Terminal and video surface [CONFIRMED code; display CONFIRMED by vehicle test]:** 1440×540. The phone stream is 1920×1080, cropped 1:1 to the centred 1440×540 (`aa_hook.h`, `altscreen_render/main.c`).
- **Plane 99 placement [CONFIRMED source]:** at the stock map origin (layout constants 108/109). In small-stage views it takes the layout's small-stage offset (80/81), never the KDK offset (`ClusterLayerController.java` lines 45–50 and 308–332).
- **KDK anchors and crops [CONFIRMED upstream measurement on MU1316 B9]:**

| Layout constant | Meaning | Classic (B9) | Sport (B9Sport) |
|---|---|---|---|
| 58/59 | in-tube anchor (101) | 1055, 207 | 984, 139 |
| 60/61 | popup anchor (102) | 1091, 110 | 1091, 110 |
| 122–125 | in-tube crop | 59, 27, 210×153 | 0, 0, 328×180 |
| 118–121 | popup crop | 59, 27, 210×153 | 59, 27, 210×153 |
| 108/109 | map origin | 0, 26 | 0, 26 |
| 80/81 | small-stage offset | 0, 0 | −476, 0 |

  Source: `upstream:docs/cluster/kdk-geometry.md`. **[INFERRED]** identical on this A4 (same `LayoutMIB2HighB9` class family); not measured here.
- **Frame rate:** `setUpdateRate(terminal 1, 30)` while a custom context is active. The encoder captures every 33 ms, so more than 30 fps never reaches the cockpit [CONFIRMED source/docs].
- **Stage and visibility signals:** the VC reports **FctID 54** (`largeMapView`, the stage) and **FctID 44** (KDK visibility) back to the HU, received via `ScreenCombiBAPListener` → `ClusterLayerController.onVcPresentation/onVcVisibility` [CONFIRMED source]. On B9 Classic the View button often raises **no** `NAV_VIEW_SIZE_CHOICE` event, so FctID 54 is the reliable stage source (`kdk-geometry.md` "⚠️" section).
- **Encoder widget sizes (`komoviewstyle.conf`):** KVS_FPK 210×153 (DSI 2) and 328×181 (DSI 3). Relevant only for the native maneuver tile [CONFIRMED upstream decompile].

### 4.4 Graphics primitives available on the unit [CONFIRMED source]
- **QNX Screen** (`screen_*`, `-lscreen`), **EGL/GLES2** (maneuver_render) and **Qualcomm OMX IL** (`OMX.qcom.video.decoder.avc`, hardware H.264). FFmpeg 6.1.5 is statically built into `altscreen_render`.
- `cluster_surface` (`upstream:common/cluster_surface.{c,h}`) is the reusable primitive: create a managed window by displayable id, detect disown via `SCREEN_PROPERTY_MANAGER_STRING`, recreate with a 100 ms backoff. **No `libdisplayinit`, no `dmdt`.**
- **Renderer priority:** both renderers run at QNX priority 15. At the default 10, compositor workers stalled renderer threads for 300–650 ms (`altscreen-mhi2q.md` §5) [CONFIRMED upstream measurement].

---

## 5. Controls

### 5.1 Steering wheel [CONFIRMED source; CONFIRMED vehicle test that all functions work]
- **Rotation (left MFW roller):** Navigation BAP `MapScale.steps` → stock `CombiBAPListener.setMapScale` → zooms the **hidden Audi map** only.
  - Android Auto has **no cluster zoom command**, so the phone map can't be zoomed from the wheel [CONFIRMED source and vehicle test]. The scale readout still moves.
- **Press:** raw DSI key **40** (`KEY_MFW_ROLLER_LEFT`, keyboard `KBD_MFW`=4 / `KBD_MFW_3GP`=8) via an extra `DSIKeyPanel` `ATTR_KEY2` listener (`SteeringWheelInputModule.java` lines 1–45).
  - Its collapsed `DDS_SELECT` copy is consumed so it doesn't reach the phone (`AndroidAuto2KeyEventsController.java` line 34).
  - Gated to the confirmed VC map tab (`MAIN_CONTEXT_NAVIGATION_MAP=20`). Short press toggles arrival time / time remaining; **long press** (key state 3) switches phone map ↔ Audi map. [Release notes 1.4; vehicle test: works.]
- **Centre DDS:** key 16. Not consumed; passes through as select.
- **Android Auto key mapping** (stock, in `AndroidAuto2KeyEventsController.java` lines 131–136): joystick N/E/S/W → AA keycodes 19/22/20/21; softkeys → 1/2.

### 5.2 Layout switching [CONFIRMED]
- The Audi **VIEW** button changes the VC stage. The VC reports it as FctID 54.
- `ClusterLayerController` → `AaClusterView.publish()` writes `/tmp/aa_cluster_view` → the hook sends 0x8009 to the phone. Layouts switch live; size and density apply at the next connect (`overview.md` "Known limits").

### 5.3 GEM page "CarPlay-RGI" [CONFIRMED: `SD-CARD/mod/menu/carplay_menu/mib2q-carplay.esd`]
Relevant buttons:
- *Android Auto cluster map ON/OFF*
- *Android Auto cluster day/night*
- *Route guidance ON/OFF*
- *HW decoder ON/OFF*
- *HW decoder 2-min delayed start*
- *Calibration grid*
- *Save logs to SD (+ verbose on)*
- *Verbose logging*
- *HW decoder probe*
- *Copy stock lsd.jxe to SD*
- *Uninstall CarPlay (restore stock)*
- *Remove this menu*

CarPlay-only buttons (SafeArea, turn card, ETA, speed limit, compass, Siri, touchpad) don't affect Android Auto.

### 5.4 Runtime marker files (Android Auto relevant) [CONFIRMED: `aa_hook.h` lines 13–52, `overview.md` "Marker files"]
`/mnt/app/root/`:
- `aa_cluster.off` (no cluster display)
- `aa_cluster.insets` (12 numbers: top/bottom/left/right for full, classic, sport)
- `aa_cluster.dpi` (80–400; default 125)
- `aa_cluster.daynight`
- `aa_cluster.navxlate_off`
- `aa_cluster.relayout_off`
- `aa_cluster.abi_trial` (test only)
- `altscreen_render.aa.hwdecode.off`
- `altscreen_render.hwdecode.delayed`

Elsewhere:
- `/mnt/app/carplay_aa.disabled`: Java bridge off; read at HMI start.
- `/mnt/app/carplay_rgd.disabled`: route guidance off.
- `/mnt/app/carplay_aa_phone_tab.off`.

---

## 6. Data availability

| Data | Origin | How it is consumed today | Status |
|---|---|---|---|
| Phone cluster map video (H.264 1920×1080, 1440×540 used) | Android Auto cluster display, through the hook | TCP tee `127.0.0.1:19821` → `altscreen_render` | **CONFIRMED** (code + vehicle) |
| Next turn, distance, street | AA NavigationStatus → legacy 0x8004/0x8005 → `DSIAndroidAuto2` | `AaBridge`/`AaRgState` → BAP | **CONFIRMED** (code + vehicle) |
| Lanes of current step | 0x8006 steps[0] | `/tmp/aa_lanes` (fixed text record) → `AaLaneFeed` | CONFIRMED code; lanes on VC **UNKNOWN** on this car (not specifically tested) |
| Final destination, remaining distance and time, ETA | 0x8006.destinations / 0x8007.destination_distances | `/tmp/aa_trip` (320-byte `AAT1`, big-endian, checksum, 30 s expiry) → `AaTripFeed` | CONFIRMED code; ETA line seen on the cockpit in tests. Upstream marks trip details "car validation pending" |
| Media cover art | `gal` writes `/tmp/gal_albumArt_<n>.png` | `AaCoverArt`/`PngCover` → 256×256 PNG in `/var/app/icab/tmp/37/` | File presence **CONFIRMED** (`local:…/002/tmp_ls.txt`); display on VC **UNKNOWN** (not tested) |
| Vehicle speed (real/displayed) | `DSICarVehicleStates.DynamicVehicleInfoHighFrequent` | `VehicleDataModule` (5 Hz), **CarPlay sessions only** | Code CONFIRMED; reading it for Android Auto/dashboards **INFERRED** feasible |
| Reverse gear, standstill, tank warning | `DSIGeneralVehicleStates` (ATTR 20, 22, 4) | same | as above |
| Range, outside temperature | `DSICarKombi` BC attributes (8, 69, 65) | same | as above |
| Trip computer (consumption, distance, averages, tank level, service/SIA) | `DSICarKombiListener` callbacks (`KombiListenerBase.java` lists `updateBC*`, `updateSIA*`) | not used by the mod | Interface **CONFIRMED** to exist in the HMI; values on this car **UNKNOWN** |
| Cluster preset/layout, display sync | `DSICarKombi` ATTR 99/100/89; `DSIKombiSync` | partly used for gating | **CONFIRMED** interface (`upstream:docs/re/firmware/dsi-carkombi.md`) |
| VC stage/visibility | BAP FctID 54 / 44 | `ClusterLayerController` | **CONFIRMED** source |
| MMI day/night design | stock `NightDayModeHandler` | `/tmp/aa_cluster_theme` (`AaClusterTheme`) | **CONFIRMED** source |
| Steering-wheel keys | `DSIKeyPanel` `ATTR_KEY2` | `SteeringWheelInputModule` | **CONFIRMED** source + vehicle |
| Engine RPM, coolant/oil temp, boost, gear position, g-forces | not observed in mod code | — | **UNKNOWN**: needs a DSI/`lsd.jxe` survey (e.g. `DynamicVehicleInfoMidFrequent`, `OilLevelData` are imported in `VehicleDataModule.java` but unused) |
| EXLAP vehicle-data service | a listener on TCP **25010** (loopback and Wi-Fi AP interface); PF anchor "EXLAP" | not used | Port **CONFIRMED** listening (`netstat_an.txt`, `pfctl_rules.txt`). Protocol, authentication and data content **UNKNOWN** |
| Android Auto car-to-phone vehicle data | — | not implemented ("Car data to the phone ❌ for AA", release notes) | **CONFIRMED** absent |

**Rules for Phase 2 data use:** DSI reads must use only `org.dsi.*` interfaces. A subclass of the stock `de.audi.remotehmi.util.dsi.DSICarKombiListenerAdapter` failed on the car with `NoClassDefFoundError` and stalled navigation ("Initialising navigation" forever); see `KombiListenerBase.java` header lines 5–8 [CONFIRMED upstream car finding].

---

## 7. Installation details and startup

### 7.1 Startup sequence after a real reboot [CONFIRMED source]
1. **Boot:** the HMI JVM starts and loads `carplay_hook.jar` from the boot classpath. Java modules register DSI listeners and the CarplayBus server on :19810.
2. **Phone connects** (USB or AAWireless): `smartphone_integrator` spawns `children.gal` = `aa_startup.sh`.
3. `aa_startup.sh`:
   - logs to `/tmp/aa_wrapper.log` and writes the owner file;
   - starts `carplay_monitor.sh`, which starts the renderers;
   - `exec`s `gal` with the hook (lines 29–58).
4. **The hook constructor:**
   - matches the profile (log `init enabled profile=P5152 gal=pinned`) and starts the tee;
   - registers the cluster sink;
   - the phone opens display 1 and frames flow.
5. `altscreen_render` decodes and writes `/tmp/altscreen_render.live`. Java's `AltScreenModule` sees it and `ScreenModule` switches to ctx 81 (or 82 with active guidance).

### 7.2 Critical operational fact: a real reboot is required [CONFIRMED vehicle and logs]
- **What happened:** after the install, briefly switching the ignition off only put the MMI into standby. The mod didn't load, and the cockpit showed only the compass.
- **Log evidence:** `local:backups/carplay_logs-2026-11-08/001` and `002`:
  - uptime about 26 min (`sloginfo_mmx.txt` 00:00:00 → 00:26:23);
  - seven `[SI] connectDevice … GAL` sessions, yet **no `/tmp/aa_wrapper.log`**;
  - no listeners on :19800/19810/19821 (`netstat_an.txt`);
  - stock `gal` running.
- **Fix:** after locking the car and letting the MMI sleep fully, followed by a cold start (Audi logo), everything worked.
- **For Phase 2:** any jar or `smartphone_integrator.json` change needs a cold boot. Verify it by the Audi start logo, or by `/tmp/aa_wrapper.log` existing.

### 7.3 Backups held locally (never in the shareable ZIP)

| Path | Content |
|---|---|
| `local:backups/MIB-backup-2026-11-08/<unit>/` (1 028 files, 148 MB, manifest `MIB-backup-2026-11-08.sha256.txt`) | M.I.B. standard backup: `MU1329-lsd.jxe` (stock HMI image, 60.6 MB), `MU1329-AppDevelopment.jar`, `MU1329-ifs-root-part2-0x00ba0000.ifs`, `MU1329-RCC_fs0.bin`, EEPROM dump, datasets, SWDL info, FEC container, HMI `rhmi` cache, partition/version info, shadow password files |
| `local:backups/carplay_logs-2026-11-08/` | mib2q-carplay *Save logs* sets 001/002 (pre-reboot state), cores, configs, and copies of `gal`/`libautoreceiver.so` |
| `local:backups/sdcard-mod-before-carplay/custom.sh` | M.I.B.'s template `mod/custom.sh`, replaced by mib2q-carplay's |
| On the unit | `smartphone_integrator.json.carplay-stock`, `dio_manager.json.carplay-stock`; M.I.B. backup part 1 in `/mnt/boardbook/MU1329` |

---

## 8. Logs and diagnostics

### 8.1 Log locations at runtime [CONFIRMED source]
- `/tmp/aa_wrapper.log`: Android Auto start-up wrapper.
- `/tmp/aa_cluster_hook.log`: key lines are `init enabled profile=…`, `uiconfig.set …`, `relayout.sent view=…`, `cluster.register ok=1`, `cluster.frame count=…`, `theme.deferred`.
- `/tmp/altscreen_render.log` and `.status`: `state=decoding`, `frames=`.
- `/tmp/carplay_java.log`: Java side, tag `AA`; `ClusterLayers apply … video=on videoDst=(x,y)`.
- `sloginfo`: a ~90 s ring dominated by video-driver noise.
- **Collecting logs:** GEM *Save logs to SD* writes `carplay_logs/NNN/` and, once per card, `carplay_logs/aa/gal` + `libautoreceiver.so`.

### 8.2 Findings from the logs we hold
- **[CONFIRMED]** Logs 001/002 show the **pre-reboot stock state** (§7.2). They contain no hook/renderer logs, so they **do not** document the working state.
- **[UNKNOWN]** No log set was captured after the successful reboot. Capturing one (Save logs twice around a route) is the first recommended Phase 2 validation step.
- **[CONFIRMED present, origin UNKNOWN]** `core/` holds crash dumps of `dio_manager`, `AppStartATF`, `navStartup`, `PNavApp`, `DRController`, `IconProviderApp`, plus `EBNavFreezeLog.*`. Timestamps are 1970 (QNX clock), so they can't be attributed to before or after the mod.
- **[CONFIRMED]** `gal` repeatedly logs `[Resource_BlockWait] Waiting for /dev/diag ..` at each connect. This is stock behaviour in this snapshot (stock `gal`).
- **[CONFIRMED]** A PF firewall is active (102 rules, `pfctl_rules.txt`). Relevant to any network-based approach.

### 8.3 Known limitations and upstream issues
- **No Android Auto cluster zoom** (protocol limitation) [CONFIRMED].
- **Dark map by default;** Light/Automatic crashed the phone app; the theme can only change at connect (#19) [CONFIRMED source].
- **#14:** MMI freeze/reboot when two decoders change state together. Fixed by `aa_yield.c` (1.4.1) [CONFIRMED source].
- **#44:** Android Auto cluster map froze about 90 s at a time on P5092/MU1329 under 1.3.1. Fixed in 1.4.
- **#48:** Android Auto drop about 2 min after boot. Fixed in 1.4.1 and listed upstream as "not yet tested in a car"; no drop was observed in the Phase 1 tests [CONFIRMED vehicle, short sessions only].
- **Waze vs Google Maps:** what is drawn into the cluster stream is decided by the phone app. Waze draws one card and no alerts; Google Maps draws a turn card plus an ETA card and zooms more smoothly. The VC's own top turn banner shows briefly per announcement (VC behaviour) [CONFIRMED vehicle].
- **Wireless:** AAWireless Gen1 works with the cluster map [CONFIRMED vehicle], although upstream lists wireless Android Auto as "not tested".

---

## 9. Safety, rollback and restrictions

### 9.1 Preserved OEM functionality [CONFIRMED vehicle test]
FM radio, Bluetooth calls, Audi navigation without a phone, parking sensors/camera, steering-wheel media controls, the voice key, VIEW layouts and the Classic layout were all checked by the user after installation, and all work.

### 9.2 Rollback ladder [CONFIRMED: `local:guide/ROLLBACK.md`, `SD-CARD/mod/install/custom.sh` lines 281–291]
1. *Android Auto cluster map OFF* (GEM) creates `aa_cluster.off`. Android Auto goes stock at the next connect.
2. *Uninstall CarPlay (restore stock)* then *Remove this menu*, then a **cold reboot**. This deletes every payload file and restores both `.carplay-stock` configs.
3. Without GEM: `extras/MIB-mods/uninstall/mod/` through M.I.B. *Run Custom Script*.
4. M.I.B. restore functions (`backup_restore` → restore) plus the local backup. Last resort, with expert guidance.

### 9.3 Recovery mechanisms built into the mod [CONFIRMED source]
- `aa_startup.sh` **always** ends in stock `gal`; a missing helper only drops the cluster extras.
- An unknown receiver keeps stock behaviour. The unpinned-`gal` crash guard `/tmp/aa_cluster.unpinned` limits a crash to one session.
- The renderer monitor restarts renderers only while its generation owns `/tmp/aa_supervisor.owner`.
- `cluster_surface_lost()` / `recreate()` handle a window disowned by the DisplayManager.
- Outside a phone session, `ClusterLayerController` restores the cached stock opacities on 101/102 so Audi navigation's KDK never stays transparent.

### 9.4 Restrictions relevant to custom dashboards
- **Never restart j9 / the HMI on a live unit:** it causes a system restart. Jar changes need a cold boot.
- **Never touch `/mnt/system` beyond the two JSON configs:** about 3.5 MB free.
- **`/mnt/app` has about 162 MB free.** Remount read-write only for the copy, use atomic `cp → chmod → mv`, then `sync`.
- **One Venus video core** is shared by the main-screen decoder and `altscreen_render`. Any extra decoder or encoder use must respect the `hw_yield`/`hw_active` protocol or it risks issue #14 (watchdog reboot).
- **QNX priorities:** run custom renderers at priority 15 or they stall.
- **One cluster context writer:** only `ScreenModule`'s worker may call `switchContext`/`setUpdateRate`. A second writer races (`display-contexts.md`).
- **Do not test while driving.** Keep the stock fallback path for any failure (the cockpit must fall back to ctx 74/80).
- **Unit tooling:** no `scp`, `sha256sum`, `tar` or `base64` (`install.md` §Traps). M.I.B. *Run Custom Script* is the supported deployment path.

---

## 10. Phase 2 opportunities and blockers

### 10.1 Directly reusable (proven on this car)
1. **Managed cluster window primitive** (`common/cluster_surface.c`): a custom renderer can own an existing displayable id. Displayable **98** is transparent and composites over a map; **99** is opaque, 1440×540, and replaces the map.
2. **Context machinery:** contexts 80/81/82 already exist in the installed `DisplayManagerMIB2High`, together with the `ScreenModule` single writer and its 72-bounce and 30 fps rate handling.
3. **Geometry engine:** `ClusterLayerController` already knows the stock map origin, the small-stage offset, the KDK anchors/crops and the VC stage (FctID 54) and visibility (FctID 44) signals.
4. **Liveness pattern:** a heartbeat file (`/tmp/altscreen_render.live`, pid plus fresh mtime) lets Java switch contexts safely and fall back when a renderer dies.
5. **Data feeds:** BAP route guidance; AA turn/lane/trip feeds (`/tmp/aa_lanes`, `/tmp/aa_trip`); cover art; DSI vehicle data via the `VehicleDataModule` pattern; day/night via `NightDayModeHandler`.
6. **GL stack:** `maneuver_render` shows EGL/GLES2 rendering, Audi fonts (`AudiTypeSemiExtended*.ttf` in `upstream:maneuver_render/resources/fonts/`), a TCP command protocol from Java (`upstream:maneuver_render/protocol.h`) and a host test harness.
7. **Build system:** Docker `qnx65-armv7-toolchain` for native code; `eclipse-temurin:8` for the jar, built against the stock jar (`upstream:docs/architecture.md` "Build & deploy"). The stock HMI image `MU1329-lsd.jxe` is in the local backup for linkage audits and decompilation.

### 10.2 Plausible architectures for Compact and Extended dashboards [INFERRED]

**Option A: a native dashboard renderer (recommended to evaluate first).**
- A new GLES renderer draws gauges/widgets into displayable 99 (opaque, full map area) or 98 (transparent overlay), fed by a Java data module over a loopback socket.
- A custom context is selected by `ScreenModule` when the user picks "Dashboard".
- *Compact* = the Classic small window between the dials; *Extended* = the wide/large map view.
- **Main risks:** coexisting with the Android Auto video on 99; a new context id, or reuse of 81/82; user-selection UX.

**Option B: a Java-side data plus overlay on top of the Android Auto map.**
- Draw dashboard elements into 98 over the phone video (ctx 82), keeping navigation visible.
- **Limits:** the 98 surface size and placement are tied to the KDK stage today (328×181 source).

**Option C: a phone-side Android Auto cluster app.**
- Not controllable from the car. The phone decides cluster content; only navigation apps get the cluster.

### 10.3 Blockers and required research

| # | Item | Status |
|---|---|---|
| B1 | Whether the HU can draw *outside* the map area (dials region) | **UNKNOWN**; evidence says no (§4.1) |
| B2 | Which displayable ids beyond 98/99 the DisplayManager accepts (103 is rejected) | **UNKNOWN**: needs the `dmdt gd`/`gs` read-out or a decompile of `libdm_modMain.so` |
| B3 | Whether a third custom context id (e.g. 83+) can be added without side effects | **UNKNOWN** |
| B4 | Selecting a dashboard from the VC/MFW without conflicting with stock tabs (key 40 is already used on the map tab) | **UNKNOWN**; needs a UX design plus `DSIKombiSync`/`IMMICombiScreenChangeManager` survey |
| B5 | Availability and refresh rates of RPM, temperatures, boost, etc. via DSI on this car | **UNKNOWN**: survey `lsd.jxe` `org.dsi.ifc.*` and log live values |
| B6 | EXLAP (:25010) as an alternative data source | **UNKNOWN** (protocol/auth) |
| B7 | GPU/CPU headroom with Android Auto video, maneuver overlay and a dashboard renderer at once | **UNKNOWN**: needs profiling on the car (`pidin`, `hogs`) |
| B8 | JVM heap headroom (`-Xmx60m`) for extra Java modules | **UNKNOWN**: measure |
| B9 | Exact panel pixel mapping of the 1440×540 terminal on AU491 (calibration grid exists for CarPlay; Android Auto via `aa_cluster.insets`) | **INFERRED**; measure with a test pattern |
| B10 | Behaviour across VC menu changes, popups (PDC, phone, warnings) and Night Vision holes while a custom context is active | **UNKNOWN** |
| B11 | Licensing: mib2q-carplay is GPL-3.0; `carplay_hook.jar` contains modified Audi/e.solutions classes not covered by the GPL (release `THIRD_PARTY_NOTICES.md`) | **CONFIRMED** constraint; decide the distribution model early |

See `PHASE2_READINESS.md` for the validation plan and `TECHNICAL_INVENTORY.md` for the file-level map.
