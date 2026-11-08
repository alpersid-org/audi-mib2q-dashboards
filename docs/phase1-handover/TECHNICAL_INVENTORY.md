# Technical Inventory – Phase 2 (Custom Virtual Cockpit Dashboards)

Companion to `PHASE2_HANDOVER.md`. Labels: **[CONFIRMED]** · **[INFERRED]** · **[UNKNOWN]**.
`local:` = `C:\Users\HP\Downloads\Audi - mib\`; `<unit>` = the redacted backup sub-folder.
`upstream:` = `kamgurgul/mib2q-carplay` at commit `b127988a68c8d690841f4a75d35717ffc01e668e` (v1.4.1).
SHA-256 values for the files listed here are in `FILE_MANIFEST_SHA256.csv`.

---

## 1. On-unit files added or changed by Phase 1

### 1.1 mib2q-carplay payload [CONFIRMED: install console output + `local:backups/carplay_logs-2026-11-08/002/hooks_ls.txt`]

| On-unit path | Type | Purpose | Source in release |
|---|---|---|---|
| `/mnt/app/root/hooks/aa_startup.sh` | sh | `children.gal` wrapper → monitor + `exec gal` with `LD_PRELOAD` | `SD-CARD/carplay/aa_startup.sh` |
| `/mnt/app/root/hooks/libaa_cluster_hook.so` | ELF ARM32 .so | Android Auto cluster display, nav translation, tee :19821, UI config 0x8009 | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/altscreen_render` | ELF ARM32 exe | OMX/FFmpeg H.264 → displayable 99 (1440×540); tees :19820 (CarPlay) / :19821 (Android Auto) | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/maneuver_render` | ELF ARM32 exe | EGL/GLES2 maneuver overlay → displayable 98; TCP :19800 from Java | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/flag_atlas.rgba` | data | Destination-flag animation atlas for `maneuver_render` | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/libcarplay_hook.so` | .so | CarPlay iAP2 hook (route guidance, cover art, phone, vehicle data); `dio_manager` only | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/libaltscreen111_mhi2q.so` | .so | CarPlay AltScreen (stream 111) hook; `dio_manager` only | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/carplay_startup.sh` | sh | `children.carplay` wrapper | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/carplay_monitor.sh`, `carplay_processes.sh` | sh | Per-generation renderer supervisor (shared by Android Auto and CarPlay) | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/carplay_cleanup.sh` | sh | CarPlay teardown (calls stock `/etc/scripts/carplay_cleanup.sh`) | `SD-CARD/carplay/` |
| `/mnt/app/root/hooks/nav_boost` | exe | Power-on navigation start priority boost | `SD-CARD/carplay/` |
| `/mnt/app/eso/hmi/lsd/jars/carplay_hook.jar` | Java 1.4 jar | HMI class overrides (contexts, layers, Android Auto bridge, route guidance/BAP, input) | `SD-CARD/carplay/` |
| `/mnt/app/eso/hmi/engdefs/mib2q-carplay.esd` | GEM esd | "CarPlay-RGI" GEM page | `SD-CARD/mod/menu/carplay_menu/mib2q-carplay.esd` |
| `/mnt/app/eso/hmi/engdefs/scripts/carplay/**` | sh (30) | GEM button scripts + `actions/*/custom.sh` | `SD-CARD/mod/menu/carplay_menu/scripts/carplay/` |

### 1.2 Configs patched in place [CONFIRMED]

| File | Change | Backup |
|---|---|---|
| `/mnt/system/etc/eso/production/smartphone_integrator.json` | `children.carplay` → `carplay_startup.sh`; `children.gal` → `aa_startup.sh` (`path /mnt/app/root/hooks`, stock envs/timeouts kept) | `…/smartphone_integrator.json.carplay-stock` |
| `/mnt/system/etc/eso/production/dio_manager.json` | iAP2 IDs added (CarPlay), `"nmea"` bit 0x08 | `…/dio_manager.json.carplay-stock` |

Patched copies captured in `local:backups/carplay_logs-2026-11-08/002/config/`. Stock `gal` child for reference: `upstream:deploy/smartphone_integrator/aa_child_stock.json` (`exec gal`, `path /mnt/app/eso/bin/apps`).

### 1.3 M.I.B. V3.8.0 changes [CONFIRMED: `local:downloads/MIB-3.8.0/extracted/…/apps/launcher`, `<unit>/MU1329-LOG.txt`]

| Target | Change |
|---|---|
| `/mnt/app/eso/hmi/engdefs/z_Launcher-sda0.esd` | symlink to `/net/mmx/fs/sda0/esd/Launcher-sda0.esd` (M.I.B. menu needs the card in SD1) |
| Persistence key `3221356557` (ns 0) | set to 1 → GEM / development mode enabled |
| `/net/rcc/mnt/efs-persist/SWDL/skipMostPopup.txt` | created |
| `/net/rcc/mnt/efs-persist/FEC/{Illegal,Withdrawn}FecContainer.fec` | deleted if present |
| `/net/rcc/mnt/efs-persist/SWDL/*.old`, `Log/service/*`, `FileCopyInfo/Launcher.info`, `MainUnit-version2.txt.SWDL.compatibility.txt` | deleted (cleanup) |
| EEPROM address `0x3f0`, `SWDL/SvmInfo.bin`, `SWDL/CfgAck.bin` | written by SVM fix (`apps/svm` lines 73–107) |
| `/net/mmx/mnt/boardbook/MU1329` | on-unit copy of M.I.B. backup part 1 |

---

## 2. Processes (runtime) [CONFIRMED: `local:…/002/pidin_ar.txt` unless marked]

| Process | Node | Role for Phase 2 |
|---|---|---|
| `j9 … -jxe:/ifs/lsd.jxe` (HMI, "lsd") | MMX | All Java patches; heap `-Xmx60m`; GEM (`-DenableGEM=true`, esd dir `/mnt/app/eso/hmi/engdefs`) |
| `/eso/bin/apps/displaymanager 2_lvds` | MMX | Owns display contexts; adopts managed windows by `ID_STRING` |
| `screen` | MMX | QNX Screen compositor |
| `./bin/apps/videoencoderservice` | MMX | Captures the cluster context, H.264 → MPEG-TS → MOST (30 fps capture) |
| `devp-iso-mmx-mib2` ×4 (`isoRX1/2`, `isoTX1/2`) | MMX | MOST/MLB isochronous transport |
| `./bin/apps/smartphone_integrator` | MMX | Spawns `gal` / `dio_manager` children from `smartphone_integrator.json` |
| `/mnt/app/eso/bin/apps/gal` | MMX | Android Auto receiver (with hook after a cold boot) |
| `aa_startup.sh` → `carplay_monitor.sh` → `altscreen_render`, `maneuver_render` | MMX | Mod processes [CONFIRMED source; not in the pre-reboot snapshot] |
| `/navigation/navStartup`, `/navigation/PNavApp` | MMX | Native navigation / cluster map renderer (`libPresentationController.so`) |
| `vpeCore` | MMX | Video processing core service |
| RCC services (`/net/rcc/…`) | RCC | Persistence, EEPROM, SWDL; M.I.B. back-end |

---

## 3. Interfaces

### 3.1 Loopback sockets [CONFIRMED source]

| Port | Server | Client | Payload |
|---|---|---|---|
| 127.0.0.1:19800 | Java `RendererServer` | `maneuver_render` | Commands: MANEUVER / PROGRESS / LANES / VISIBLE_AREA (`upstream:maneuver_render/protocol.h`) |
| 127.0.0.1:19810 | Java `CarplayBus` | `libcarplay_hook.so` (CarPlay) | Framed bus protocol (`upstream:hook/framework/bus_protocol.h`) |
| 127.0.0.1:19820 | `libaltscreen111_mhi2q.so` | `altscreen_render` | Annex-B H.264, CarPlay cluster |
| 127.0.0.1:19821 | `libaa_cluster_hook.so` (in `gal`) | `altscreen_render` | Annex-B H.264 1920×1080, Android Auto cluster |
| 25010 (loopback + Wi-Fi AP) | stock (EXLAP) | — | **UNKNOWN** protocol; listening confirmed in `netstat_an.txt` |

### 3.2 Shared-memory files in `/tmp` (= `/dev/shmem`, fixed-length records, no directories) [CONFIRMED source]

| File | Writer → Reader | Format |
|---|---|---|
| `/tmp/aa_cluster_view` | Java `AaClusterView` → hook | 8 bytes: `full   \n` / `classic\n` / `sport  \n` |
| `/tmp/aa_cluster_theme` | Java `AaClusterTheme` → hook | 6 bytes: `day  \n` / `night\n` |
| `/tmp/aa_lanes` | hook → Java `AaLaneFeed` | fixed-length text record |
| `/tmp/aa_trip` | hook → Java `AaTripFeed` | 320 B big-endian `AAT1` + checksum, 30 s expiry |
| `/tmp/gal_albumArt_<n>.png` | stock `gal` → Java `AaCoverArt` | PNG |
| `/tmp/altscreen_render.live` / `.status` | renderer → Java `AltScreenModule` | `pid=`, mtime heartbeat / `state=`, `frames=` |
| `/tmp/altscreen_render.hw_yield` / `.hw_active` | hook ↔ renderer | uptime ms / `pid=` (video-core arbitration) |
| `/tmp/aa_supervisor.owner` | `aa_startup.sh` → monitor | generation PID |
| `/tmp/aa_cluster.unpinned` | hook | unpinned-`gal` crash guard |
| `/tmp/aa_wrapper.log`, `/tmp/aa_cluster_hook.log`, `/tmp/altscreen_render.log`, `/tmp/carplay_java.log` | various | logs |

### 3.3 Android Auto protocol messages touched by the hook [CONFIRMED source]

| ID | Direction | Use |
|---|---|---|
| Version request/response | both | ask 4.3, rewrite answer to 1.7 |
| VideoConfiguration field 11 (UiConfig) | HU → phone | insets + theme at connect |
| 0x8009 UpdateUiConfigRequest | HU → phone | per-view safe area |
| 0x8006 NavigationState / 0x8007 NavigationCurrentPosition | phone → HU | translated in place to 0x8004 / 0x8005 |

### 3.4 HMI / DSI / BAP interfaces referenced [CONFIRMED in source]

| Interface | Used for |
|---|---|
| `IDisplayManagerKombiControl` (`switchContext`, `setUpdateRate`, `setPosition`, `setCropping`, `setOpacity`) | cluster planes |
| `DSIAndroidAuto2` (via `AndroidAuto2ListenerDistributor`, `AndroidAuto2NavHandler`, `AndroidAuto2KeyEventsController`) | Android Auto state, next turn, keys |
| `DSIKeyPanel` `ATTR_KEY2` | steering-wheel raw keys |
| `DSICarVehicleStates`, `DSIGeneralVehicleStates`, `DSICarKombi`, `DSIKombiSync` | vehicle and cluster data |
| `IMMICombiScreenChangeManager` (`MAIN_CONTEXT_NAVIGATION_MAP=20`) | confirmed VC tab |
| BAP Navigation FctIDs: 19 route text, 44 KDK visibility, 45 scale readout, 54 stage | VC exchange (`upstream:docs/rgd/bap-fctids.md`) |

---

## 4. Upstream source files most relevant to Phase 2 (read at commit b127988)

| File | Why it matters |
|---|---|
| `common/cluster_surface.{c,h}` | Managed cluster window primitive (create, disown detect, recreate) |
| `java_patch/de/audi/tghu/fwhmi/DisplayManagerMIB2High.java` | Full cluster context table; where new contexts would be declared |
| `java_patch/com/luka/carplay/core/ScreenModule.java` | The single context-switch worker (bounce 72, 30 fps) |
| `java_patch/com/luka/carplay/core/AltScreenModule.java` | Heartbeat → context selection |
| `java_patch/com/luka/carplay/cluster/ClusterLayerController.java` | Plane geometry from the live stock `Layout`; FctID 44/54 handling |
| `java_patch/com/luka/carplay/aa/*.java` | Android Auto bridge, view, theme, trip, lanes, cover art |
| `java_patch/com/luka/carplay/core/VehicleDataModule.java`, `KombiListenerBase.java` | DSI vehicle-data access pattern (org.dsi interfaces only) |
| `java_patch/com/luka/carplay/core/SteeringWheelInputModule.java` | Raw MFW key observation and gating |
| `aa_hook/aa_hook.{c,h}`, `aa_uiconfig.c`, `aa_navxlate.c`, `aa_trip.c`, `aa_tee.c`, `aa_yield.c` | Android Auto cluster display internals |
| `altscreen_render/main.c`, `hw_decode.c`, `video_gles.c`, `view.c` | Decoder/presenter on displayable 99 |
| `maneuver_render/platform_qnx.c`, `render.c`, `protocol.h`, `server.c`, `visible_area.h`, `resources/fonts/` | GLES renderer template on displayable 98 |
| `deploy/smartphone_integrator/*` | Child wrappers + stock child JSON |
| `scripts/build_*.sh`, `scripts/stock_env.sh`, `stock/README.md` | Docker build; stock-jar requirements |
| `docs/cluster/*.md`, `docs/re/firmware/*.md`, `docs/android-auto/*.md`, `docs/deploy/*.md` | Reverse-engineering notes (statuses stated in each file's front matter) |

---

## 5. Local artifacts (read-only; NOT in the shareable ZIP)

| Path | Content | Sensitivity |
|---|---|---|
| `local:downloads/mib2q-carplay-1.4.1/mib2q-carplay-1.4.1.zip` + `extracted/` | Release as installed | contains proprietary-derived jar |
| `local:downloads/MIB-3.8.0/MIB-V3.8.0-source.zip` + `extracted/` | M.I.B. as installed | — |
| `local:backups/MIB-backup-2026-11-08/<unit>/MU1329-lsd.jxe` | Stock HMI J9 image (60.6 MB) – main resource for DSI/class surveys | proprietary |
| `…/<unit>/MU1329-AppDevelopment.jar` | Stock GEM jar | proprietary |
| `…/<unit>/MU1329-ifs-root-part2-0x00ba0000.ifs`, `MU1329-RCC_fs0.bin` | RCC flash images | proprietary, unit-specific |
| `…/<unit>/MU1329-EEProm.*`, `FEC/*`, `MU1329-shadow*`, `MU1329-allversions.txt`, `MU1329-LOG.txt` | EEPROM, feature codes, password hashes, identity | **sensitive** (VIN in FEC container; serials; coding) |
| `…/<unit>/HMI/rhmi/**` | HMI remote/smartphone cache | possibly personal |
| `local:backups/carplay_logs-2026-11-08/{001,002}/` | Pre-reboot diagnostic snapshots | **sensitive** (`pidin_ar.txt` j9 line holds VIN/IMSI; `ifconfig.txt` MACs) |
| `local:backups/carplay_logs-2026-11-08/aa/{gal,libautoreceiver.so}` | Stock Android Auto receiver binaries | proprietary |
| `local:backups/MIB-backup-2026-11-08.sha256.txt` | Backup checksum list | contains redacted-folder name |
| `local:CHECKLIST.md`, `local:guide/*.md` | Phase 1 checklist, in-car guides, rollback | shareable after review |
