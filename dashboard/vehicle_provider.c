/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-08 VehicleDataProvider, renderer side: reads the DBV1 record that the HMI
 * DashboardModule (java_patch/com/luka/carplay/dashboard) publishes from existing
 * pipelines. Read-only file access; no sockets, DSI or vehicle commands.
 * Layout is documented in DashboardModule.java and docs/custom-dashboards/vehicle-provider.md. */
#define _POSIX_C_SOURCE 199309L
#include "dashboard.h"
#include <limits.h>
#include <string.h>
#include <time.h>

#define MAGIC 0x44425631u /* "DBV1" */
#define DEST_OFF 64
#define DEST_END 192

static uint32_t be32(const unsigned char *b)
{
    return (uint32_t)b[0] << 24 | (uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | b[3];
}

static uint64_t be64(const unsigned char *b)
{
    return (uint64_t)be32(b) << 32 | be32(b + 4);
}

static int32_t sbe32(const unsigned char *b)
{
    uint32_t u = be32(b);
    return u <= (uint32_t)INT32_MAX ? (int32_t)u : (int32_t)(u - 0x80000000u) + INT32_MIN;
}

static int zero(const unsigned char *b, size_t from, size_t to)
{
    size_t i;
    for (i = from; i < to; ++i) if (b[i]) return 0;
    return 1;
}

/* System-clock sample time -> sample on the caller's clock. 0 = absent part. */
static void stamp(db_sample *s, uint64_t sample_wall, uint64_t wall_now, uint64_t now)
{
    uint64_t age;
    memset(s, 0, sizeof(*s));
    if (!sample_wall) return;                        /* MISSING */
    s->source = DB_SOURCE_VEHICLE;
    if (sample_wall > wall_now) { s->availability = DB_INVALID; return; }
    age = wall_now - sample_wall;
    if (age > now) { s->availability = DB_INVALID; return; } /* older than our clock epoch */
    s->availability = DB_AVAILABLE;
    s->sampled_ms = now - age;                       /* formatter applies the 30 s limit */
}

int db_vehicle_parse(const unsigned char *b, size_t len, uint64_t wall_now, uint64_t now,
                     db_snapshot *out)
{
    db_snapshot s;
    uint32_t hash = 0x811c9dc5u;
    uint64_t written, temp_ms, range_ms, nav_ms;
    int32_t temp, range, meters, seconds;
    size_t i, end;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!b || len != DB_VEHICLE_RECORD_SIZE || be32(b) != MAGIC) return 0;
    for (i = 0; i < DB_VEHICLE_RECORD_SIZE - 4; ++i) hash = (hash ^ b[i]) * 16777619u;
    if (hash != be32(b + DB_VEHICLE_RECORD_SIZE - 4)) return 0;
    if (!zero(b, 57, 60) || !zero(b, DEST_END, DB_VEHICLE_RECORD_SIZE - 4) || b[56] > 2)
        return 0;
    if (!be32(b + 4) || !be32(b + 60)) return 0;     /* sequence and session start at 1 */
    written = be64(b + 8);
    /* The writer runs every 500 ms; a missing writer must suppress the dashboard. */
    if (!written || written > wall_now || wall_now - written > DB_MAX_AGE_MS) return 0;
    temp = sbe32(b + 16); range = sbe32(b + 20);
    meters = sbe32(b + 24); seconds = sbe32(b + 28);
    temp_ms = be64(b + 32); range_ms = be64(b + 40); nav_ms = be64(b + 48);
    for (end = DEST_OFF; end < DEST_END && b[end]; ++end) { }
    if (end - DEST_OFF >= DB_TEXT_CAP || !zero(b, end, DEST_END)) return 0;
    /* A sample time after the record was written is corrupt. */
    if (temp_ms > written || range_ms > written || nav_ms > written) return 0;

    memset(&s, 0, sizeof(s));
    if (temp != INT32_MIN && temp_ms) {
        stamp(&s.temperature.sample, temp_ms, wall_now, now);
        s.temperature.decicelsius = temp;
    }
    if (range != -1 && range_ms) {
        stamp(&s.range.sample, range_ms, wall_now, now);
        s.range.meters = range;
    }
    if (nav_ms) {
        stamp(&s.navigation.sample, nav_ms, wall_now, now);
        s.navigation.remaining_meters = meters;
        s.navigation.remaining_seconds = seconds;
        memcpy(s.navigation.destination, b + DEST_OFF, end - DEST_OFF);
    }
    /* media: never supplied (no verified Android Auto source); stays MISSING. */
    *out = s;
    return 1;
}

static uint64_t realtime_ms(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_REALTIME, &t) != 0 || t.tv_sec < 0) return 0;
    return (uint64_t)t.tv_sec * 1000u + (uint64_t)t.tv_nsec / 1000000u;
}

int db_vehicle_read(void *context, uint64_t now_ms, db_snapshot *out)
{
    db_vehicle_source *src = context;
    const char *path = src && src->path ? src->path : DB_VEHICLE_PATH;
    unsigned char buf[DB_VEHICLE_RECORD_SIZE + 1];
    uint32_t session, sequence;
    uint64_t wall;
    size_t n;
    int bad;
    FILE *f;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    f = fopen(path, "rb");
    if (!f) return 0;                                /* no record: dashboard suppressed */
    n = fread(buf, 1, sizeof(buf), f);
    bad = ferror(f);
    if (fclose(f) != 0) bad = 1;
    if (bad) return 0;
    /* Review R3: sample the clock only after the complete record is in memory, so a
     * record published while we opened/read the file is not judged "future". Ages are
     * then measured slightly late and mapped onto the earlier caller now_ms: errs old. */
    wall = src && src->wall_ms ? src->wall_ms() : realtime_ms();
    if (!wall || !db_vehicle_parse(buf, n, wall, now_ms, out)) return 0;
    if (!src) return 1;
    session = be32(buf + 60);
    sequence = be32(buf + 4);
    if (session != src->last_session) {
        /* New writer session (HMI restart, disconnect/reconnect, re-enable). */
        src->last_session = session;
        src->last_sequence = sequence;
        src->last_advance_wall = wall;
        return 1;
    }
    if (sequence < src->last_sequence) goto reject;  /* replay/corruption in one session */
    if (sequence > src->last_sequence) {
        src->last_sequence = sequence;
        src->last_advance_wall = wall;
        return 1;
    }
    /* Same record as before: the writer is alive only if it is recent. */
    if (wall >= src->last_advance_wall &&
        wall - src->last_advance_wall <= DB_VEHICLE_WRITER_TIMEOUT_MS) return 1;
reject:
    memset(out, 0, sizeof(*out));
    return 0;
}
