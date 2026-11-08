#!/bin/sh
# M.I.B. -> Advanced Settings -> Run Custom Script.
# QNX 6.5 /bin/sh is ksh (pdksh) - this script stays inside its portable subset.
# Copy into /mod/ on the M.I.B. SD; resources live in /mod/carplay/.
#
# Install:
#   - our files: the hook .so, maneuver_render, its flag atlas, the carplay_*.sh
#     scripts and the jar. No stock file is replaced. Two layouts, both accepted:
#       * flat: the release assets dropped straight into carplay/ - each known
#         name goes to its fixed on-unit path (see flat_dest);
#       * tree: carplay/root/<on-unit path> copied onto "/".
#   - in-place runtime patches (per-unit / stock-dependent), done here:
#       * smartphone_integrator.json  - replace the "carplay" child by path, and the
#                                       "gal" (Android Auto) child when the release has it
#       * dio_manager.json            - register every iAP2 message ID the hook relies on
# A persistent transaction directory stages a full update and retains recovery
# snapshots until activation succeeds. Interrupted installs require `recover`.
# Stock/config and conflicting-JAR backups survive upgrades for uninstall.
# Copyright (c) 2026 LuKa (@LuKa_dev)
set -u
PATH=/proc/boot:/bin:/usr/bin:/usr/sbin:/sbin:/mnt/app/armle/bin:/mnt/app/armle/usr/bin
export PATH
unset LD_PRELOAD

case $0 in */*) D=${0%/*} ;; *) D=. ;; esac
D=$(cd "$D" && pwd) || exit 1

# GEM runs on MMX. Forward a manual RCC launch (the MMX-local check below is false
# once we are on MMX, so this never loops).
if [ ! -d /mnt/app/eso/hmi/lsd ] && [ -d /net/mmx/mnt/app/eso/hmi/lsd ]; then
    exec on -f mmx /bin/sh "$D/custom.sh" "$@"
fi

RES=${CP_INSTALL_RES:-$D/carplay}  # resources (carplay_child.json, flat release assets);
                               # the GEM CarPlay menu points this at the release on the card
ROOT=$RES/root                 # optional "/" tree
ACTION=${1:-install}

HOOKS=/mnt/app/root/hooks
JARS=/mnt/app/eso/hmi/lsd/jars
# Flat release asset -> on-unit path. Anything else in carplay/ is ignored.
flat_dest() {
    case $1 in
        libcarplay_hook.so|maneuver_render|flag_atlas.rgba) echo "$HOOKS/$1" ;;
        carplay_startup.sh|carplay_monitor.sh|carplay_processes.sh|carplay_cleanup.sh) echo "$HOOKS/$1" ;;
        carplay_hook.jar) echo "$JARS/$1" ;;
        libaltscreen111_mhi2q.so|altscreen_render) echo "$HOOKS/$1" ;;  # optional AltScreen
        libaa_cluster_hook.so|aa_startup.sh) echo "$HOOKS/$1" ;;          # optional Android Auto
        nav_boost) echo "$HOOKS/$1" ;;                                   # optional power-on navigation boost
        *) return 1 ;;
    esac
}
# Optional assets. Present => installed; absent => skipped, base RGI install
# unaffected. AltScreen (cluster VIDEO renderer + CarPlay hook, see
# docs/deploy/altscreen-mhi2q.md) and Android Auto (gal launcher + cluster hook,
# see docs/android-auto/overview.md).
OPTIONAL_ASSETS="libaltscreen111_mhi2q.so altscreen_render libaa_cluster_hook.so aa_startup.sh nav_boost"

CFG=/mnt/system/etc/eso/production/smartphone_integrator.json
DIO=/mnt/system/etc/eso/production/dio_manager.json

mode_for() {
    case $1 in
        *.so|*.so.*)        echo 755 ;;
        */maneuver_render)  echo 755 ;;
        */altscreen_render) echo 755 ;;
        */nav_boost)        echo 755 ;;
        *.sh)               echo 755 ;;
        *)                  echo 644 ;;  # atlas, .jar
    esac
}
# count occurrences of $2 in $1 (ksh-safe, no external tools)
count_char() { n=0; rest=$1; while :; do case $rest in *"$2"*) rest=${rest#*"$2"}; n=$((n+1)) ;; *) break ;; esac; done; echo "$n"; }

backup_once() { [ -e "$2" ] || cp -p "$1" "$2" || { echo "FAILED backup $2"; return 1; }; }

TX=$HOOKS/.carplay-transaction
tx_active=0

# Snapshot before touching any destination. Never follow symlinks or overwrite
# directories. The journal lives on /mnt/app, not volatile /tmp, for power loss.
tx_save() {
    tx_path=$1
    case $tx_path in *'|'*) echo "FAILED unsupported destination $tx_path"; return 1 ;; esac
    if [ -L "$tx_path" ] || { [ -e "$tx_path" ] && [ ! -f "$tx_path" ]; }; then
        echo "FAILED unsafe destination $tx_path"; return 1
    fi
    tx_count=$((tx_count + 1))
    tx_state=absent
    if [ -e "$tx_path" ]; then
        cp -p "$tx_path" "$TX/$tx_count.old" || return 1
        tx_state=present
    fi
    printf '%s|%s|%s\n' "$tx_count" "$tx_state" "$tx_path" >> "$TX/journal" || return 1
}

tx_restore() {
    # A truncated journal must fail closed before changing any destination.
    tx_expected=$(cat "$TX/count" 2>/dev/null || :)
    case $tx_expected in ''|0|*[!0-9]*) echo "FAILED invalid recovery count: $TX"; return 1 ;; esac
    tx_checked=0
    while IFS='|' read -r tx_id tx_state tx_path; do
        tx_checked=$((tx_checked + 1))
        [ "$tx_id" = "$tx_checked" ] || { echo "FAILED invalid recovery entry"; return 1; }
        case $tx_path in /*) ;; *) echo "FAILED invalid recovery destination"; return 1 ;; esac
        case $tx_state in
        present) [ -f "$TX/$tx_id.old" ] && [ ! -L "$TX/$tx_id.old" ] || {
            echo "FAILED recovery snapshot missing: $tx_id"; return 1; } ;;
        absent) ;;
        *) echo "FAILED invalid recovery state"; return 1 ;;
        esac
    done < "$TX/journal"
    [ "$tx_checked" = "$tx_expected" ] || { echo "FAILED incomplete recovery journal; retain $TX"; return 1; }
    tx_failed=0
    while IFS='|' read -r tx_id tx_state tx_path; do
        [ -n "$tx_id" ] || continue
        case $tx_state in
        present)
            cp -p "$TX/$tx_id.old" "$tx_path.carplay-restore.$$" &&
                mv -f "$tx_path.carplay-restore.$$" "$tx_path" || tx_failed=1 ;;
        absent) rm -f "$tx_path" || tx_failed=1 ;;
        *) tx_failed=1 ;;
        esac
        rm -f "$tx_path.carplay-new.$$"
    done < "$TX/journal"
    [ "$tx_failed" = 0 ] || { echo "FAILED rollback; retain $TX and run recover after resolving storage errors"; return 1; }
    sync || return 1
    rm -rf "$TX" || return 1
    echo "Previous installation restored."
}

tx_exit() {
    tx_rc=$?
    trap - 0 HUP INT TERM
    if [ "$tx_active" = 1 ]; then
        tx_active=0
        if [ -f "$TX/armed" ]; then
            tx_restore || tx_rc=1
        else
            rm -rf "$TX" || tx_rc=1
        fi
        [ "$tx_rc" != 0 ] || tx_rc=1
    fi
    rm -f "$LIST"
    exit "$tx_rc"
}

recover() {
    [ -d "$TX" ] && [ ! -L "$TX" ] || { echo "No transaction to recover."; return 0; }
    tx_pid=$(cat "$TX/pid" 2>/dev/null || :)
    case $tx_pid in ''|*[!0-9]*) ;; *)
        if kill -0 "$tx_pid" 2>/dev/null; then
            echo "FAILED installer PID $tx_pid may still be running; recovery refused"; return 1
        fi ;;
    esac
    if [ -f "$TX/armed" ]; then
        [ -f "$TX/journal" ] || { echo "FAILED recovery journal missing: $TX"; return 1; }
        tx_restore || return 1
    else
        rm -rf "$TX" || return 1    # no destination was changed before arming
    fi
    sync
}

restore_conflicts() {
    for conflict in NavActiveIgnore.jar navignore_audi.jar navignore_vw.jar; do
        conflict=$JARS/$conflict
        [ -f "$conflict.carplay-stock" ] || continue
        [ ! -e "$conflict" ] && [ ! -L "$conflict" ] || {
            echo "FAILED restore $conflict: destination exists; backup retained"; return 1; }
        mv -f "$conflict.carplay-stock" "$conflict" || return 1
    done
}

check_conflict_restore() {
    for conflict in NavActiveIgnore.jar navignore_audi.jar navignore_vw.jar; do
        conflict=$JARS/$conflict
        if [ -f "$conflict.carplay-stock" ] && { [ -e "$conflict" ] || [ -L "$conflict" ]; }; then
            echo "FAILED restore $conflict: destination exists; backup retained"; return 1
        fi
    done
}

# The release assets, dropped flat into carplay/.  All or nothing: a partly copied
# release would pair a new carplay_startup.sh with an old monitor, so it stops here.
FLAT_ASSETS="libcarplay_hook.so maneuver_render flag_atlas.rgba carplay_startup.sh
carplay_monitor.sh carplay_processes.sh carplay_cleanup.sh carplay_hook.jar"

# Payload as "source|destination" lines into $1.  Flat assets are checked by name,
# never by walking the card.  The optional root/ tree needs find: QNX fs-dos cannot
# stat ".." inside the folders of some FAT cards, so find reports
# "./dir/..: Filename too long" and exits 1 although the list is complete.  Tolerate
# exactly that error; anything else (or an empty list) still fails.
list_payload() {
    : > "$1" || { echo "FAILED create $1"; return 1; }
    missing=
    for a in $FLAT_ASSETS; do
        if [ -f "$RES/$a" ]; then printf '%s|%s\n' "$RES/$a" "$(flat_dest "$a")" >> "$1"
        else missing="$missing $a"; fi
    done
    if [ -s "$1" ] && [ -n "$missing" ]; then
        echo "FAILED release incomplete, missing in $RES:$missing"; rm -f "$1"; return 1
    fi
    # Optional AltScreen assets: add each only if present; never fail on absence.
    for a in $OPTIONAL_ASSETS; do
        if [ -f "$RES/$a" ]; then
            printf '%s|%s\n' "$RES/$a" "$(flat_dest "$a")" >> "$1"
            echo "  (+ optional asset: $a)"
        fi
    done
    if [ -d "$ROOT" ]; then
        ( cd "$ROOT" && find . -type f > "$1.tree" ) 2> "$1.err"
        if grep -v '/\.\.: Filename too long' "$1.err" | grep -q .; then
            echo "FAILED listing payload"; cat "$1.err"; rm -f "$1" "$1.tree" "$1.err"; return 1
        fi
        while IFS= read -r f; do
            case $f in */._*|*/.DS_Store) continue ;; esac   # macOS junk from a Mac-written card
            printf '%s|%s\n' "$ROOT/${f#./}" "/${f#./}"
        done < "$1.tree" >> "$1"
        rm -f "$1.tree" "$1.err"
    fi
    [ -s "$1" ] || { echo "no payload in $RES (release files or root/ tree)"; rm -f "$1"; return 1; }
}

# ---- smartphone_integrator.json: replace a child by path ----------------------
# patch_child <child> <fragment> <launcher>: replace children.<child> with the
# fragment. <launcher> names our wrapper, so a config that already runs it is not
# taken as the stock backup.
patch_child() {
    CHILD=$1; FRAG=$2; LAUNCHER=$3
    [ -f "$CFG" ]  || { echo "  WARN no SI config at $CFG - skipping json"; return 0; }
    [ -f "$FRAG" ] || { echo "  WARN no ${FRAG##*/} resource - skipping json"; return 0; }
    tmp=$CFG.carplay-new.$$
    : > "$tmp" || { echo "FAILED create $tmp"; return 1; }
    state=copy; hits=0; depth=0
    while IFS= read -r line || [ -n "$line" ]; do
        if [ "$state" = copy ]; then
            case $line in
                *"\"$CHILD\""*:*)
                    set -f; set -- $line; set +f
                    compact=; for part in "$@"; do compact=$compact$part; done
                    [ "$compact" = "\"$CHILD\":{" ] || { echo "  WARN unsupported $CHILD layout"; rm -f "$tmp"; return 1; }
                    hits=$((hits+1)); state=skip; depth=0 ;;
                *) printf '%s\n' "$line" >> "$tmp"; continue ;;
            esac
        fi
        opens=$(count_char "$line" '{'); closes=$(count_char "$line" '}')
        depth=$((depth + opens - closes))
        if [ "$depth" -le 0 ]; then
            printf '        "%s": ' "$CHILD" >> "$tmp"
            cat "$FRAG" >> "$tmp"
            rest=${line##*\}}                                 # keep the trailing comma;
            [ -z "$rest" ] || printf '%s\n' "$rest" >> "$tmp"  # nothing else: no blank line
            state=copy
        fi
    done < "$CFG"
    [ "$hits" = 1 ] && [ "$state" = copy ] || { echo "  WARN expected one $CHILD child (hits=$hits)"; rm -f "$tmp"; return 1; }
    o=$(grep -c '"exec"' "$CFG"); n=$(grep -c '"exec"' "$tmp")
    [ "$o" = "$n" ] || { echo "  WARN SI child count changed ($o->$n)"; rm -f "$tmp"; return 1; }
    if ! grep -q 'carplay_startup.sh' "$CFG" && ! grep -q 'aa_startup.sh' "$CFG"; then
        backup_once "$CFG" "$CFG.carplay-stock" || { rm -f "$tmp"; return 1; }
    fi
    grep -q "$LAUNCHER" "$CFG" || [ -e "$CFG.carplay-stock" ] || {
        echo "  WARN $CHILD child left as is: no $CFG.carplay-stock to restore it from"; rm -f "$tmp"; return 1; }
    chmod 644 "$tmp" && mv -f "$tmp" "$CFG" || { rm -f "$tmp"; return 1; }
    echo "  SI json: $CHILD child patched"
}

patch_json() {
    patch_child carplay "$RES/carplay_child.json" carplay_startup.sh || return 1
    # Android Auto: only when the release carries the gal launcher and its fragment.
    if [ -f "$RES/aa_startup.sh" ] && [ -f "$RES/aa_child.json" ]; then
        patch_child gal "$RES/aa_child.json" aa_startup.sh
    fi
}

# ---- dio_manager.json: register every iAP2 message ID the hook relies on ------
# The Cinemo iAP2 SDK passes only listed messages. The hook's Identify patch makes
# iOS offer route guidance but does not touch these lists. The file carries "##"
# comment lines, so it is edited as text: IDs are appended before the list's "]".
add_ids() {   # $1 = line holding one list, rest = IDs; prints the new line
    l=$1; shift
    for id in "$@"; do
        case $l in *"\"$id\""*) continue ;; esac
        head=${l%%]*}; tail=${l#*]}
        case $head in *'[') l=$head'"'$id'"]'$tail ;; *) l=$head', "'$id'"]'$tail ;; esac
    done
    printf '%s\n' "$l"
}
del_ids() {   # $1 = line holding one list, rest = IDs that belong to the other list; prints the new line
    l=$1; shift
    for id in "$@"; do
        q="\"$id\""
        case $l in
            *", $q"*) l=${l%%", $q"*}${l#*", $q"} ;;
            *"$q, "*) l=${l%%"$q, "*}${l#*"$q, "} ;;
            *"$q"*)   l=${l%%"$q"*}${l#*"$q"} ;;
        esac
    done
    printf '%s
' "$l"
}
# Every iAP2 message the hook relies on, stock ones included: each is added only when missing,
# so a dio_manager.json that something else trimmed is repaired too; an ID found in the wrong
# list is moved (0x5203 among the received IDs kept CarPlay from starting, issue #23).  iOS identifies a feature
# only when every REQUIRED message of its group is declared, and rejects the group otherwise
# (accessoryd 27.0 __checkIdentificationInfo.kMsg*List, same rule in 26.1); required = *:
#   now playing (cover art)  sent 0x5000* 0x5002*          received 0x5001*
#   power (phone battery)    sent 0xAE00* 0xAE02*          received 0xAE01*
#   location (PASCD)         sent 0xFFFB*                  received 0xFFFA* 0xFFFC*
#   call state / comms       sent 0x4154* 0x4157*          received 0x4155* 0x4158*
#   call control             sent 0x415A 0x415B 0x415C 0x415D 0x415F 0x4160
#                            (InitiateCall for favorites without a number, Accept/End/Swap/Hold/Mute)
#   list updates             sent 0x4170*                  received 0x4171*
#   route guidance           sent 0x5200* 0x5203*          received 0x5201* 0x5202* 0x5204
#   vehicle status           sent 0xA101*                  received 0xA100* 0xA102*
DIO_SENT_IDS="0x5000 0x5002 0xAE00 0xAE02 0xFFFB 0x4154 0x4157 0x415A 0x415B 0x415C 0x415D 0x415F 0x4160 0x4170 0x5200 0x5203 0xA101"
DIO_RECV_IDS="0x5001 0xAE01 0xFFFA 0xFFFC 0x4155 0x4158 0x4171 0x5201 0x5202 0x5204 0xA100 0xA102"
# "nmea":"0xNNNN" is the NMEA bitmask dio advertises in Identify (LocationInformationComponent).
# Bit 0x08 = PASCD (VehicleSpeedData): the hook supplies the wheel-speed sentences.
add_nmea_speed() {   # $1 = line; prints the new line
    l=$1; pre=${l%%\"nmea\":\"*}; v=${l#*\"nmea\":\"}; rest=${v#*\"}; v=${v%%\"*}
    case $v in 0x[0-9A-Fa-f]*) printf '%s"nmea":"0x%04X"%s\n' "$pre" $((v | 8)) "$rest" ;; *) printf '%s\n' "$l" ;; esac
}
patch_dio() {
    [ -f "$DIO" ] || { echo "  WARN no $DIO - route guidance will not arrive"; return 0; }
    tmp=$DIO.carplay-new.$$; : > "$tmp" || { echo "FAILED create $tmp"; return 1; }
    sent=0; recv=0; changed=0
    while IFS= read -r line || [ -n "$line" ]; do
        case $line in
            *'##'*) new=$line ;;
            *'"MessagesSentByAccessory":['*']'*)
                sent=$((sent+1)); new=$(add_ids "$(del_ids "$line" $DIO_RECV_IDS)" $DIO_SENT_IDS) ;;
            *'"MessagesReceivedFromDevice":['*']'*)
                recv=$((recv+1)); new=$(add_ids "$(del_ids "$line" $DIO_SENT_IDS)" $DIO_RECV_IDS) ;;
            *'"nmea":"0x'*)
                new=$(add_nmea_speed "$line") ;;
            *) new=$line ;;
        esac
        [ "$new" = "$line" ] || { line=$new; changed=1; }
        printf '%s\n' "$line" >> "$tmp"
    done < "$DIO"
    if [ "$sent" != 1 ] || [ "$recv" != 1 ]; then
        echo "  WARN dio_manager.json: lists sent=$sent recv=$recv (expected 1/1); left as-is"; rm -f "$tmp"; return 1
    fi
    n=0; want=0
    for id in $DIO_SENT_IDS $DIO_RECV_IDS; do want=$((want+1)); grep -q "\"$id\"" "$tmp" && n=$((n+1)); done
    [ "$n" = "$want" ] || { echo "  WARN dio_manager.json: only $n/$want IDs after edit; left as-is"; rm -f "$tmp"; return 1; }
    if [ "$changed" = 0 ]; then echo "  dio_manager.json IDs already registered"; rm -f "$tmp"; return 0; fi
    backup_once "$DIO" "$DIO.carplay-stock" || { rm -f "$tmp"; return 1; }
    chmod 644 "$tmp" && mv -f "$tmp" "$DIO" || { rm -f "$tmp"; return 1; }
    echo "  dio_manager.json: all hook message IDs registered (nmea +PASCD)"
}

echo "custom.sh: CarPlay $ACTION"
echo "Remounting app and system read-write..."
mount -uw /mnt/app    2>/dev/null || true
mount -uw /mnt/system 2>/dev/null || true

case $ACTION in
recover) recover; exit $? ;;
install|uninstall) ;;
*) echo "usage: custom.sh [install|uninstall|recover]"; exit 2 ;;
esac
[ ! -e "$TX" ] && [ ! -L "$TX" ] || {
    echo "FAILED pending transaction $TX; run custom.sh recover first"; exit 1; }

# Payload as "source|destination" lines (/tmp is /dev/shmem: no directories there).
LIST=/tmp/carplay_files.$$
list_payload "$LIST" || exit 1

case $ACTION in
install)
    mkdir -p "$HOOKS" || exit 1
    (umask 077; mkdir "$TX") || { echo "FAILED acquire transaction $TX"; exit 1; }
    tx_active=1; tx_count=0
    trap tx_exit 0
    trap 'exit 1' HUP INT TERM
    echo $$ > "$TX/pid" || exit 1
    : > "$TX/journal" || exit 1
    # Back up every affected path, including backups created during this attempt.
    while IFS='|' read -r f dest; do
        mkdir -p "${dest%/*}" || exit 1
        tx_save "$dest" || exit 1
        cp -p "$f" "$TX/$tx_count.new" || exit 1
        chmod $(mode_for "$dest") "$TX/$tx_count.new" || exit 1
    done < "$LIST"
    for dest in "$CFG" "$DIO" "$CFG.carplay-stock" "$DIO.carplay-stock"; do
        tx_save "$dest" || exit 1
    done
    for conflict in NavActiveIgnore.jar navignore_audi.jar navignore_vw.jar; do
        conflict=$JARS/$conflict
        tx_save "$conflict" && tx_save "$conflict.carplay-stock" || exit 1
        if [ -f "$conflict" ] && [ -f "$conflict.carplay-stock" ]; then
            cmp -s "$conflict" "$conflict.carplay-stock" || {
                echo "FAILED conflicting backup differs: $conflict; resolve before installing"; exit 1; }
        fi
    done
    echo "$tx_count" > "$TX/count" || exit 1
    sync || exit 1
    : > "$TX/armed" || exit 1
    sync || exit 1
    # Activate only after all payload files and recovery snapshots are staged.
    tx_index=0
    while IFS='|' read -r f dest; do
        tx_index=$((tx_index + 1))
        dir=${dest%/*}
        mkdir -p "$dir" || { echo "FAILED mkdir $dir"; rm -f "$LIST"; exit 1; }
        tmp=$dest.carplay-new.$$
        cp -p "$TX/$tx_index.new" "$tmp" || { echo "FAILED copy $tmp"; rm -f "$tmp" "$LIST"; exit 1; }
        chmod $(mode_for "$dest") "$tmp" || { echo "FAILED chmod $tmp"; rm -f "$tmp" "$LIST"; exit 1; }
        mv -f "$tmp" "$dest" || { echo "FAILED activate $dest"; rm -f "$tmp" "$LIST"; exit 1; }
        echo "  $dest"
    done < "$LIST"
    rm -f "$LIST"
    # M.I.B.'s "NavActiveIgnore" (navignore_audi/_vw.jar, installed as NavActiveIgnore.jar)
    # replaces org.dsi.ifc.carplay.AppState so getAppStateID()/getOwner() always return 0:
    # the HMI no longer sees which resources CarPlay owns, which our lifecycle relies on.
    # Preserve these pre-existing modifications so uninstall can restore them.
    for j in NavActiveIgnore.jar navignore_audi.jar navignore_vw.jar; do
        j=$JARS/$j
        [ -e "$j" ] || continue
        backup_once "$j" "$j.carplay-stock" && rm -f "$j" || exit 1
        echo "  backed up and disabled $j (conflicts with CarPlay app state)"
    done
    patch_json || { echo "FAILED smartphone integrator configuration; rolling back"; exit 1; }
    patch_dio || { echo "FAILED dio configuration; rolling back"; exit 1; }
    sync || exit 1
    # The durable commit marker distinguishes a complete install from interrupted
    # activation. Recovery of a committed transaction only removes its journal.
    mv "$TX/armed" "$TX/committed" || exit 1
    sync || { tx_active=0; echo "FAILED commit sync; retain $TX and run recover"; exit 1; }
    tx_active=0
    rm -rf "$TX" || { echo "FAILED journal cleanup; run recover"; exit 1; }
    echo "DONE (install). Reboot the HU to load."
    ;;
uninstall)
    check_conflict_restore || exit 1
    while IFS='|' read -r f dest; do
        rm -f "$dest" || { echo "FAILED remove $dest"; exit 1; }
    done < "$LIST"
    rm -f "$LIST"
    # restore in-place patched configs
    if [ -e "$CFG.carplay-stock" ]; then mv -f "$CFG.carplay-stock" "$CFG" || exit 1; fi
    if [ -e "$DIO.carplay-stock" ]; then mv -f "$DIO.carplay-stock" "$DIO" || exit 1; fi
    restore_conflicts || exit 1
    sync || exit 1
    echo "DONE (uninstall). Reboot the HU."
    ;;
*)
    rm -f "$LIST"
    echo "usage: custom.sh [install|uninstall]"; exit 2
    ;;
esac
