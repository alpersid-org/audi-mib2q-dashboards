# Phase 1 → Phase 2 handover (user-supplied, 2026-11-08)

Prepared by the user's Phase 1 investigation of their own car and laptop backup,
and supplied to this project on 2026-11-08 as `phase2-handover_2026-11-08.zip`.
The three documents below are stored **unchanged**; they are evidence, not
project-authored text. Their labels (CONFIRMED / INFERRED / UNKNOWN) are the
handover author's.

| File | SHA-256 | Content |
| --- | --- | --- |
| [PHASE2_HANDOVER.md](PHASE2_HANDOVER.md) | `6d4220098ae3b103d4879b43b8d7841b6bf32ec38c32434e19f49eda1ef2b84f` | Full technical handover (hardware, installed software, AA→VC architecture, rendering, controls, data, install/startup, logs, safety, opportunities/blockers) |
| [TECHNICAL_INVENTORY.md](TECHNICAL_INVENTORY.md) | `e946b0071ec20c514f1d8c2ecbdcde5a833673361cf9e96f427d097caf5e94ee` | Files on the unit, processes, sockets, shared files, message IDs, source map |
| [PHASE2_READINESS.md](PHASE2_READINESS.md) | `c364282063a3dd855936bf8e9d11b55ff21c128844aa5c1bbc2ad89411562992` | Proven capabilities, limits, open questions, risks, ordered validation plan |

These digests match the entries in the handover's own `FILE_MANIFEST_SHA256.csv`.
ZIP received: SHA-256 `ecb6840d745f1a7f17d66b6eb704fc409733200b4c61582ab90b446d27e218c2`
(its companion `.sha256` file was not supplied, so this is the digest as received).

**Not committed:** `FILE_MANIFEST_SHA256.csv` (SHA-256
`112e1345d2f0ae2b13ce124cb3ad2d01217aa8c1528cb171c21e27f11774a93a`, 242 entries).
It inventories the user's private laptop backup and logs, including entries for
sensitive files (EEPROM dump, password-hash files, a log holding VIN/IMSI). It is
only meaningful next to that backup, so it stays with the user.

**Privacy check on receipt:** besides the author's scan, the three documents were
searched for VIN-, IMSI-, MAC-, e-mail- and IP-shaped strings: none found except
`127.0.0.1`. Unit-specific backup folder names are redacted as `<unit>`.

## What this changes in the project

- **Phase 1 IS installed and vehicle-tested** (upstream mib2q-carplay **1.4.1
  release**, via M.I.B. V3.8.0, 2026-11-08): Android Auto cluster map works wired
  and wireless (AAWireless Gen1), in Classic and the wide map view; OEM functions
  checked. This supersedes "Phase 1 NOT INSTALLED" in the earlier target profile.
- The installed runtime is the **upstream release**, not this fork's build. This
  fork's Java runtime contains P2-08 dashboard changes, so installing it would
  replace a known-good jar; that stays a separate, authorized decision.
- The unit's `gal` is byte-identical to upstream profile **P5152** (pinned), with
  the shared MU0918 receiver; no new porting is needed for the existing hook.
- **Surface constraint (INFERRED, high confidence):** the head unit supplies only
  the VC **map/KDK area** as video; dials and fixed VC widgets cannot be redrawn.
  Compact/Extended dashboards therefore target the map area (Classic window /
  wide view). Only displayables 98 and 99 are proven; 103 is rejected.
- No post-reboot (working-state) log set exists yet; capturing one is step 1 of
  the handover's validation plan.
- **Licensing (CONFIRMED constraint):** the jar contains modified Audi /
  e.solutions classes not covered by the GPL; decide the distribution model
  before publishing builds.

Reconciled in: [target-profile.md](../custom-dashboards/target-profile.md),
[completion-plan.md](../custom-dashboards/completion-plan.md),
[requirements.md](../custom-dashboards/requirements.md), status/HANDOFF and the audit.
