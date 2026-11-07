/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-FINAL tests for the P2-08 VehicleDataProvider reader (dashboard/vehicle_provider.c):
 * DBV1 validation, absent parts, clock mapping, review R3 interleaving, writer
 * session/sequence liveness, manager integration and, with a path argument, a record
 * produced by the real Java DashboardModule.encode (cross-language fixture). */
#include "../dashboard/dashboard.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define W UINT64_C(1700000000000) /* wall-clock base, ms */

typedef struct {
    uint32_t seq, session;
    uint64_t written, temp_ms, range_ms, nav_ms;
    int32_t temp, range, meters, seconds;
    unsigned owner;
    const char *dest;
} rec_spec;

static void put32(unsigned char *b, uint32_t v)
{
    b[0] = (unsigned char)(v >> 24); b[1] = (unsigned char)(v >> 16);
    b[2] = (unsigned char)(v >> 8); b[3] = (unsigned char)v;
}

static void put64(unsigned char *b, uint64_t v)
{
    put32(b, (uint32_t)(v >> 32)); put32(b + 4, (uint32_t)v);
}

static void seal(unsigned char *b)
{
    uint32_t h = 0x811c9dc5u;
    int i;
    for (i = 0; i < DB_VEHICLE_RECORD_SIZE - 4; ++i) h = (h ^ b[i]) * 16777619u;
    put32(b + DB_VEHICLE_RECORD_SIZE - 4, h);
}

static void build(unsigned char *b, const rec_spec *r)
{
    memset(b, 0, DB_VEHICLE_RECORD_SIZE);
    put32(b, 0x44425631u);
    put32(b + 4, r->seq);
    put64(b + 8, r->written);
    put32(b + 16, (uint32_t)r->temp); put32(b + 20, (uint32_t)r->range);
    put32(b + 24, (uint32_t)r->meters); put32(b + 28, (uint32_t)r->seconds);
    put64(b + 32, r->temp_ms); put64(b + 40, r->range_ms); put64(b + 48, r->nav_ms);
    b[56] = (unsigned char)r->owner;
    put32(b + 60, r->session);
    if (r->dest) memcpy(b + 64, r->dest, strlen(r->dest));
    seal(b);
}

static rec_spec good(void)
{
    rec_spec r;
    r.seq = 5; r.session = 0x1234abcdu;
    r.written = W; r.temp_ms = W - 100; r.range_ms = W - 200; r.nav_ms = W - 300;
    r.temp = 125; r.range = 620000; r.meters = 42500; r.seconds = 2280;
    r.owner = 2; r.dest = "City Centre";
    return r;
}

static int has_text(const db_frame *f, const char *s)
{
    size_t i;
    for (i = 0; i < f->count; ++i)
        if (f->commands[i].kind == DB_DRAW_TEXT && !strcmp(f->commands[i].text, s)) return 1;
    return 0;
}

static void parse_valid(void)
{
    unsigned char b[DB_VEHICLE_RECORD_SIZE];
    rec_spec r = good();
    db_snapshot s;
    db_widget_view v;
    build(b, &r);
    memset(&s, 'X', sizeof(s));
    assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s));
    assert(s.temperature.sample.availability == DB_AVAILABLE);
    assert(s.temperature.sample.source == DB_SOURCE_VEHICLE);
    assert(s.temperature.sample.sampled_ms == 100000 - 100);  /* age mapped to caller clock */
    assert(s.range.sample.sampled_ms == 100000 - 200 && s.range.meters == 620000);
    assert(s.navigation.sample.sampled_ms == 100000 - 300);
    assert(s.navigation.remaining_meters == 42500 && s.navigation.remaining_seconds == 2280);
    assert(!strcmp(s.navigation.destination, "City Centre"));
    assert(s.media.sample.availability == DB_MISSING);        /* media never supplied */
    db_widget_format(DB_TEMPERATURE, &s, 100000, &v);
    assert(v.status == DB_VALUE_OK && !strcmp(v.value, "12.5 C") && !strcmp(v.badge, "UNVERIFIED"));
    db_widget_format(DB_NAV_TIME, &s, 100000, &v);
    assert(!strcmp(v.value, "38 min") && !strcmp(v.badge, "UNVERIFIED"));
    db_widget_format(DB_MEDIA, &s, 100000, &v);
    assert(v.status == DB_VALUE_MISSING && !strcmp(v.badge, "UNAVAILABLE"));
}

static void parse_absent_and_clock(void)
{
    unsigned char b[DB_VEHICLE_RECORD_SIZE];
    rec_spec r = good();
    db_snapshot s;
    db_widget_view v;
    /* Absent parts: INT32_MIN temperature, -1 range, nav time 0. */
    r.temp = INT32_MIN; r.range = -1; r.nav_ms = 0; r.meters = r.seconds = -1; r.dest = NULL;
    build(b, &r);
    assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s));
    assert(s.temperature.sample.availability == DB_MISSING);
    assert(s.range.sample.availability == DB_MISSING);
    assert(s.navigation.sample.availability == DB_MISSING);
    /* Value present but time 0 is absent too. */
    r = good(); r.temp_ms = 0; build(b, &r);
    assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s));
    assert(s.temperature.sample.availability == DB_MISSING);
    /* Navigation with only an estimate time and -1 parts stays a valid group. */
    r = good(); r.seconds = -1; r.dest = NULL; build(b, &r);
    assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s));
    db_widget_format(DB_NAVIGATION, &s, 100000, &v);
    assert(v.status == DB_VALUE_OK && !strcmp(v.value, "42.5 km"));
    db_widget_format(DB_DESTINATION, &s, 100000, &v);
    assert(v.status == DB_VALUE_MISSING);
    /* Age older than the caller's clock epoch cannot be mapped: INVALID. */
    r = good(); build(b, &r);
    assert(db_vehicle_parse(b, sizeof(b), W, 50, &s));
    assert(s.temperature.sample.availability == DB_INVALID);
    /* 30 s boundary: sample exactly 30 s old is OK, 1 ms more is STALE. */
    r = good(); r.written = W; r.temp_ms = W - DB_MAX_AGE_MS; build(b, &r);
    assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s));
    db_widget_format(DB_TEMPERATURE, &s, 100000, &v); assert(v.status == DB_VALUE_OK);
    r.temp_ms = W - DB_MAX_AGE_MS - 1; build(b, &r);
    assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s));
    db_widget_format(DB_TEMPERATURE, &s, 100000, &v); assert(v.status == DB_VALUE_STALE);
    /* Non-ASCII UTF-8 destination is shown with '?' by the formatter. */
    r = good(); r.dest = "Z\xc3\xbcrich"; build(b, &r);
    assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s));
    db_widget_format(DB_DESTINATION, &s, 100000, &v);
    assert(!strcmp(v.value, "Z??rich"));
}

static void parse_rejects(void)
{
    unsigned char b[DB_VEHICLE_RECORD_SIZE], big[DB_VEHICLE_RECORD_SIZE + 1];
    rec_spec r;
    db_snapshot s;
    char longdest[100];
#define REJECT(stmt) do { r = good(); build(b, &r); stmt; memset(&s, 'X', sizeof(s)); \
        assert(!db_vehicle_parse(b, sizeof(b), W, 100000, &s)); \
        assert(s.temperature.sample.availability == DB_MISSING && !s.navigation.destination[0]); } while (0)
    REJECT(b[0] = 'X'; seal(b));                         /* magic */
    REJECT(b[100] ^= 1);                                 /* checksum */
    REJECT(b[57] = 1; seal(b));                          /* padding 57..59 */
    REJECT(b[59] = 1; seal(b));
    REJECT(b[200] = 1; seal(b));                         /* padding 192..251 */
    REJECT(b[251] = 1; seal(b));
    REJECT(b[56] = 3; seal(b));                          /* owner > 2 */
    REJECT(put32(b + 60, 0); seal(b));                   /* session 0 */
    REJECT(put32(b + 4, 0); seal(b));                    /* sequence 0 */
    REJECT(put64(b + 8, 0); seal(b));                    /* written 0 */
    REJECT(put64(b + 8, W + 1); seal(b));                /* written in the future */
    REJECT(put64(b + 8, W - DB_MAX_AGE_MS - 1); put64(b + 32, W - DB_MAX_AGE_MS - 2);
           put64(b + 40, W - DB_MAX_AGE_MS - 2); put64(b + 48, W - DB_MAX_AGE_MS - 2); seal(b));
    REJECT(put64(b + 32, W + 1); seal(b));               /* sample newer than written */
    REJECT(put64(b + 40, W + 1); seal(b));
    REJECT(put64(b + 48, W + 1); seal(b));
    REJECT(memset(b + 64, 'A', 128); seal(b));           /* unterminated destination */
    REJECT(b[80] = 'x'; seal(b));                        /* byte after the NUL */
    memset(longdest, 'D', 96); longdest[96] = 0;         /* 96 bytes >= DB_TEXT_CAP */
    REJECT(memcpy(b + 64, longdest, 96); seal(b));
    /* exactly 95 bytes is accepted */
    r = good(); build(b, &r); memset(b + 64, 'D', 95); memset(b + 64 + 95, 0, 1); seal(b);
    assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s) && strlen(s.navigation.destination) == 95);
    /* size */
    r = good(); build(b, &r);
    assert(!db_vehicle_parse(b, sizeof(b) - 1, W, 100000, &s));
    memcpy(big, b, sizeof(b)); big[DB_VEHICLE_RECORD_SIZE] = 0;
    assert(!db_vehicle_parse(big, sizeof(big), W, 100000, &s));
    assert(!db_vehicle_parse(NULL, 0, W, 100000, &s));
    assert(!db_vehicle_parse(b, sizeof(b), W, 100000, NULL));
    /* record 30 s old exactly is accepted */
    r = good(); r.written = W - DB_MAX_AGE_MS; r.temp_ms = r.range_ms = r.nav_ms = r.written;
    build(b, &r); assert(db_vehicle_parse(b, sizeof(b), W, 100000, &s));
#undef REJECT
}

/* ---- file reader, R3 interleaving and liveness ---------------------------- */

static char rec_path[512];
static uint64_t fake_wall;
static int publish_during_clock;
static rec_spec published_late;

static void write_record(const rec_spec *r)
{
    unsigned char b[DB_VEHICLE_RECORD_SIZE];
    FILE *f = fopen(rec_path, "wb");
    assert(f);
    build(b, r);
    assert(fwrite(b, 1, sizeof(b), f) == sizeof(b));
    assert(!fclose(f));
}

static uint64_t wall_cb(void)
{
    /* R3: a publication racing with the reader's clock sample. The reader must
     * already hold the complete record, so this newer record cannot be read now. */
    if (publish_during_clock) { publish_during_clock = 0; write_record(&published_late); }
    return fake_wall;
}

static void reader(const char *dir)
{
    db_vehicle_source src;
    db_snapshot s;
    rec_spec r = good();
    FILE *f;
    snprintf(rec_path, sizeof(rec_path), "%s/carplay_dashboard", dir);
    memset(&src, 0, sizeof(src));
    src.path = rec_path; src.wall_ms = wall_cb;
    remove(rec_path);
    fake_wall = W;
    assert(!db_vehicle_read(&src, 100000, &s));                 /* missing file */
    f = fopen(rec_path, "wb"); assert(f); fputs("short", f); assert(!fclose(f));
    assert(!db_vehicle_read(&src, 100000, &s));                 /* short file */

    /* R3: reader sampled the clock at W in the old order; now it reads first. The
     * record on disk was written at W - 50; during the clock sample a new record
     * with written W + 1 is published. It must not cause a "future" rejection. */
    r.written = W - 50; r.temp_ms = r.range_ms = r.nav_ms = W - 60;
    write_record(&r);
    published_late = r; published_late.seq = r.seq + 1; published_late.written = W + 1;
    published_late.temp_ms = published_late.range_ms = published_late.nav_ms = W + 1;
    publish_during_clock = 1;
    assert(db_vehicle_read(&src, 100000, &s));
    assert(src.last_session == r.session && src.last_sequence == r.seq);
    /* Next read sees the late record; it is valid once the clock passes it. */
    fake_wall = W + 1;
    assert(db_vehicle_read(&src, 100000, &s) && src.last_sequence == r.seq + 1);

    /* Liveness: unchanged sequence accepted up to 2 s, then rejected. */
    r = good(); r.session = 77; r.seq = 1; r.written = W + 1000;
    r.temp_ms = r.range_ms = r.nav_ms = W + 1000;
    write_record(&r);
    fake_wall = W + 1000;
    assert(db_vehicle_read(&src, 100000, &s) && src.last_session == 77);   /* new session */
    fake_wall = W + 1000 + DB_VEHICLE_WRITER_TIMEOUT_MS;
    assert(db_vehicle_read(&src, 100000, &s));                              /* boundary */
    fake_wall = W + 1000 + DB_VEHICLE_WRITER_TIMEOUT_MS + 1;
    memset(&s, 'X', sizeof(s));
    assert(!db_vehicle_read(&src, 100000, &s));                             /* writer silent */
    assert(s.temperature.sample.availability == DB_MISSING);
    /* Advancing sequence revives it. */
    r.seq = 2; r.written = fake_wall; r.temp_ms = r.range_ms = r.nav_ms = fake_wall;
    write_record(&r);
    assert(db_vehicle_read(&src, 100000, &s) && src.last_sequence == 2);
    /* Backwards sequence within the session is rejected. */
    r.seq = 1; write_record(&r);
    assert(!db_vehicle_read(&src, 100000, &s));
    assert(src.last_sequence == 2);
    /* A new session (reconnect / HMI restart) is accepted even with a lower sequence. */
    r.session = 78; r.seq = 1; write_record(&r);
    assert(db_vehicle_read(&src, 100000, &s) && src.last_session == 78);
    /* NULL context: stateless default read of the default path is allowed. */
    assert(db_vehicle_read(NULL, 100000, NULL) == 0);

    /* Manager integration: READY with UNVERIFIED badges, cleared when the writer stops. */
    {
        db_provider p;
        db_manager m;
        db_config c;
        db_vehicle_source msrc;
        memset(&msrc, 0, sizeof(msrc));
        msrc.path = rec_path; msrc.wall_ms = wall_cb;
        p.read = db_vehicle_read; p.context = &msrc;
        db_manager_init(&m, &p);
        db_config_default(&c); c.enabled = 1; c.layout = DB_EXTENDED;
        db_manager_set_config(&m, &c);
        r = good(); r.session = 90; r.seq = 1; r.written = fake_wall;
        r.temp_ms = r.range_ms = r.nav_ms = fake_wall;
        write_record(&r);
        assert(db_manager_update(&m, 100000, 800, 252, (cr_rect_t){0, 0, 800, 252}));
        assert(has_text(db_manager_frame(&m), "UNVERIFIED") && has_text(db_manager_frame(&m), "12.5 C"));
        assert(!has_text(db_manager_frame(&m), "MOCK"));
        fake_wall += DB_VEHICLE_WRITER_TIMEOUT_MS + 1;
        assert(!db_manager_update(&m, 100000, 800, 252, (cr_rect_t){0, 0, 800, 252}));
        assert(db_manager_get_status(&m) == DB_MANAGER_OFF_PROVIDER);
        assert(db_manager_frame(&m)->count == 0);
        remove(rec_path);
        assert(!db_manager_update(&m, 100000, 800, 252, (cr_rect_t){0, 0, 800, 252}));
    }
}

/* Record written by the real Java DashboardModule.encode (DashboardModuleTest). */
static void java_fixture(const char *path)
{
    unsigned char b[DB_VEHICLE_RECORD_SIZE + 1];
    db_snapshot s;
    db_widget_view v;
    size_t n;
    FILE *f = fopen(path, "rb");
    assert(f);
    n = fread(b, 1, sizeof(b), f);
    assert(!fclose(f) && n == DB_VEHICLE_RECORD_SIZE);
    /* Values fixed in DashboardModuleTest.writeFixture. */
    assert(db_vehicle_parse(b, n, UINT64_C(1700000000500), 100000, &s));
    assert(s.temperature.decicelsius == 125 && s.temperature.sample.sampled_ms == 100000 - 100);
    assert(s.range.meters == 620000 && s.range.sample.sampled_ms == 100000 - 200);
    assert(s.navigation.remaining_meters == 42500 && s.navigation.remaining_seconds == 2280);
    assert(s.navigation.sample.sampled_ms == 100000 - 400);     /* oldest part (estimate) */
    assert(!strcmp(s.navigation.destination, "Z\xc3\xbcrich Hbf"));
    assert(b[56] == 2 && b[60] == 0x12 && b[61] == 0x34 && b[62] == 0x56 && b[63] == 0x78);
    db_widget_format(DB_RANGE, &s, 100000, &v);
    assert(!strcmp(v.value, "620 km") && !strcmp(v.badge, "UNVERIFIED"));
    puts("dashboard_vehicle_test: Java-encoded DBV1 fixture parsed by C PASS");
}

int main(int argc, char **argv)
{
    assert(argc == 2 || argc == 3); /* scratch dir [, Java fixture] */
    parse_valid(); parse_absent_and_clock(); parse_rejects(); reader(argv[1]);
    puts("dashboard_vehicle_test: DBV1 validation/rejections, absent parts, clock mapping, R3 interleaving, session/sequence liveness, manager integration PASS");
    if (argc == 3) java_fixture(argv[2]);
    return 0;
}
