# Target vehicle profile — P5092 / MU1329

> **Phase 1 handover (2026-11-08, supersedes "NOT INSTALLED"):** [Phase 1 handover](../phase1-handover/README.md) shows the
> upstream mib2q-carplay **1.4.1 release** installed via M.I.B. V3.8.0 and vehicle-
> tested: AA cluster map wired and wireless (AAWireless Gen1), Classic and wide view,
> OEM functions intact. The unit's `gal` matches upstream profile P5152 (pinned).
> Surface constraint (INFERRED, high confidence): dashboards can occupy only the VC
> map area; only displayables 98/99 are proven. No post-reboot log set exists yet.

Updated: 2026-11-08. Source: the user's explicit target configuration in this
conversation. These values are accepted planning inputs. No head-unit inspection,
installation or physical compatibility test was performed by the agent.

## Supplied configuration

| Field | User-supplied value |
| --- | --- |
| Vehicle | Audi A4 B9 2017 |
| Firmware train | `MHI2Q_ER_AUG22_P5092` |
| MU software | `1329` |
| Media driver | `DEV_MMX2QC_AU_ER_G22_201PROD-1` |
| Instrument cluster | Audi Virtual Cockpit |
| VC modes to investigate | Classic; Extended Map |
| Phone | Samsung Galaxy S23 Ultra |
| Android Auto | Working on MMI |
| Wireless adapter | Original AAWireless |
| Adapter hardware | `cs307-e2-prod` |
| Adapter firmware | `3.5.0` |
| Phase 1 installation | **INSTALLED and vehicle-tested 2026-11-08** (upstream 1.4.1 release via M.I.B. V3.8.0; see the Phase 1 handover). Earlier value "NOT INSTALLED" superseded. |

Classic and Extended Map describe the **OEM VC modes**. Compact and Extended
describe our **custom dashboard layouts**. Their names do not establish a pixel
rectangle, displayable ID, context mapping or equivalence with upstream's Sport/
wide mode names. Evaluate both dashboard layouts in both intended OEM modes;
record unsupported combinations explicitly rather than infer a mapping.

## What this establishes

- C02 target identity inventory is partially complete: firmware/MU/media-driver,
  cluster/modes, phone/adapter and installation status are now supplied.
- Stock MMI Android Auto is reported working. This is a baseline observation to
  preserve; it does not establish that the original project is installed.
- Phase 1 is explicitly not performed. Its installation status must not be
  described as unknown or complete. Enhanced cluster output and recovery behavior
  remain untested on this setup.

## Repository compatibility evidence and limits

The inherited [README supported-unit table](../../README.md#supported-units)
contains reports for P5092/MU1329 and A4 B9/2017 (source line 94 at the reviewed
baseline). This supports investigating this firmware as a candidate; reports for
other units/releases do not certify this unit's new custom dashboards.

The [wireless-adapter table](../../README.md#tested-wireless-adapters) names
**AAWireless TWO+** with firmware qualifications (source line 120). It does not
validate original AAWireless hardware `cs307-e2-prod` on firmware `3.5.0` for a
secondary cluster stream or the richer navigation feed. No adapter firmware
upgrade, replacement or TWO+ profile is inferred from that report.

The receiver [profile table and matching code](../../aa_hook/aa_hook.c) use actual
`libautoreceiver` symbols/layouts and `gal` build evidence. MU1329 or the media-
driver text alone does not prove their ABI. The table's profile names are not a
firmware-name allowlist: neither add a P5092 row by guessing offsets nor reject
it solely because that label is absent. Keep existing ABI safety checks and
avoid trial/unpinned overrides as a shortcut.

## Remaining evidence

| Item | Current state | Required evidence |
| --- | --- | --- |
| Original project installed/validated | **INSTALLED** (upstream 1.4.1 release), AA cluster map vehicle-tested; OEM checks passed | Post-reboot (working-state) log set; recovery ladder exercised only as documented, not tested |
| Exact receiver/HMI/graphics ABI | Receiver: unit `gal` byte-identical to profile P5152 (pinned), MU0918 `libautoreceiver.so` (handover §2.3). HMI/graphics ABI not established | Exact stock artifacts/API/symbol identities and provenance; keep proprietary files outside Git |
| Cluster dimensions and ownership | Cluster KI_FPK_AU491; terminal surface 1440×540 displays correctly; VC draws dials itself, HU supplies the map area only (INFERRED); exact visible rectangle unmeasured | Rectangle, clipping, scale, priorities and OEM restoration evidence per Classic/Extended Map |
| Original adapter's enhanced projection behavior | Cluster map works with AAWireless Gen1 (vehicle test, short sessions); long-session behaviour (#48) not established | Behavior of the original upstream solution with this exact adapter/version; a working main MMI screen is insufficient |
| Real dashboard signals | Not observed through this project | Source meanings/units/validity, update/service-loss behavior and session ownership |
| Target resource/storage limits | Not measured | Baseline measurements and a reviewed recovery/storage budget |
| Dashboard on VC | Missing implementation; not tested | C03-C09 gates in the completion plan |

## Updated execution order

1. Continue C01 CI diagnosis and C05's independent host controller/settings work.
2. Complete C02 baseline **preparation** using this identity: review the original
   solution's exact package/build compatibility and recovery needs before any
   separately authorized installation. The already reviewed upstream v1.4.1
   (`b127988a68c8d690841f4a75d35717ffc01e668e`) is a candidate pinned runtime
   baseline, not an assertion that its distributed JAR matches this unit.
3. Establish Phase 1 separately, then observe the original solution's behavior
   with this adapter and both intended OEM modes. Phase 1 uses the original
   runtime payload; the fork's Java build already contains dashboard changes
   even with dashboards default OFF, so it is not automatically the original
   Phase 1 baseline.
4. Preserve the installer audit findings when planning Phase 1. Original upstream
   installation scripts do not acquire this fork's conflict-backup/journal fixes
   merely because the runtime version is 1.4.1. Inventory and preserve existing
   files and establish recovery; if a reviewed installer-only correction is used
   with upstream runtime, document that deviation explicitly.
5. Use real baseline/stock evidence to complete C03 surface feasibility and C04
   signal acceptance before target dashboard routing and activation. If the
   adapter lacks a particular optional signal, omit that widget; do not assume
   that every dashboard signal depends on secondary map-video support.

This profile does not authorize installing, flashing, running commands on the
head unit or changing the adapter. It is not a compiled activation profile and
does not remove the QNX/default-OFF guard. Full work packages and acceptance:
[completion-plan.md](completion-plan.md).
