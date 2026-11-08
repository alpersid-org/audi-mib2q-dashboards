/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-C05 tests: runtime controller end to end on the host,
 * DBV1 record file -> db_vehicle_read -> manager -> frame -> fake output backend. */
#include "../dashboard/runtime.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define W UINT64_C(1700000000000)

/* ---- DBV1 writer (layout: DashboardModule.java / vehicle-provider.md) ---- */
static char path[512];
static uint64_t wall;
static uint32_t seq;
static unsigned wall_reads;

static void put32(unsigned char *b, uint32_t v)
{ b[0] = (unsigned char)(v >> 24); b[1] = (unsigned char)(v >> 16); b[2] = (unsigned char)(v >> 8); b[3] = (unsigned char)v; }
static void put64(unsigned char *b, uint64_t v) { put32(b, (uint32_t)(v >> 32)); put32(b + 4, (uint32_t)v); }

static void publish(uint32_t session, int32_t temp)
{
    unsigned char b[DB_VEHICLE_RECORD_SIZE];
    uint32_t h = 0x811c9dc5u;
    int i;
    FILE *f;
    memset(b, 0, sizeof(b));
    put32(b, 0x44425631u); put32(b + 4, ++seq); put64(b + 8, wall);
    put32(b + 16, (uint32_t)temp); put32(b + 20, 620000);
    put32(b + 24, 42500); put32(b + 28, 2280);
    put64(b + 32, wall); put64(b + 40, wall); put64(b + 48, wall);
    b[56] = 2; put32(b + 60, session);
    memcpy(b + 64, "City Centre", 11);
    for (i = 0; i < DB_VEHICLE_RECORD_SIZE - 4; ++i) h = (h ^ b[i]) * 16777619u;
    put32(b + DB_VEHICLE_RECORD_SIZE - 4, h);
    f = fopen(path, "wb"); assert(f);
    assert(fwrite(b, 1, sizeof(b), f) == sizeof(b)); assert(!fclose(f));
}
static uint64_t wall_cb(void) { wall_reads++; return wall; }

/* ---- fake output backend: enforces the ownership contract ---- */
typedef struct {
    int owned, visible;
    int acquires, granted, draws, clears, releases;
    int fail_acquire, fail_draw;
    int width, height;
    db_frame last;
} fake;

static int has_text(const db_frame *f, const char *s)
{
    size_t i;
    for (i = 0; i < f->count; ++i)
        if (f->commands[i].kind == DB_DRAW_TEXT && !strcmp(f->commands[i].text, s)) return 1;
    return 0;
}
static int f_acquire(void *c, int w, int h, cr_rect_t a)
{
    fake *k = c; (void)a;
    assert(!k->owned);                 /* never acquired twice */
    k->acquires++;
    if (k->fail_acquire) { k->fail_acquire--; return 0; }
    k->owned = 1; k->granted++; k->width = w; k->height = h;
    return 1;
}
static int f_draw(void *c, const db_frame *f)
{
    fake *k = c;
    assert(k->owned && db_frame_valid(f) && f->count > 0);
    assert(!has_text(f, "MOCK"));     /* no mock data in the production path */
    k->draws++;
    if (k->fail_draw) { k->fail_draw--; k->visible = 1; return 0; }
    k->visible = 1; k->last = *f;
    return 1;
}
static void f_clear(void *c) { fake *k = c; assert(k->owned); k->clears++; k->visible = 0; }
static void f_release(void *c)
{ fake *k = c; assert(k->owned && !k->visible); k->releases++; k->owned = 0; }

static const db_runtime_profile WIDE = { 800, 252, {0, 0, 800, 252} };

static db_config cfg(int enabled, db_layout layout)
{
    db_config c; db_config_default(&c); c.enabled = enabled; c.layout = layout; return c;
}

int main(int argc, char **argv)
{
    fake k;
    db_output_backend be;
    db_runtime rt;
    db_config c;
    db_runtime_profile bad = { 210, 153, {0, 0, 210, 153} };
    uint64_t t = 100000;
    int i, draws;
    assert(argc == 2);
    snprintf(path, sizeof(path), "%s/carplay_dashboard_rt", argv[1]);
    remove(path);
    memset(&k, 0, sizeof(k));
    be.context = &k; be.acquire = f_acquire; be.draw = f_draw;
    be.clear = f_clear; be.release = f_release;
    wall = W;

    /* Default: disabled; nothing touched even with a valid record. */
    db_runtime_init(&rt, &be, path, wall_cb);
    db_runtime_set_profile(&rt, &WIDE);
    publish(1, 125);
    assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED);
    assert(k.acquires == 0 && k.draws == 0);

    /* Enabled but unsupported profile: no acquire. NULL profile likewise. */
    c = cfg(1, DB_COMPACT); db_runtime_apply_config(&rt, &c);
    db_runtime_set_profile(&rt, &bad);
    assert(db_runtime_tick(&rt, t) == DB_RT_UNSUPPORTED && k.acquires == 0);
    db_runtime_set_profile(&rt, NULL);
    assert(db_runtime_tick(&rt, t) == DB_RT_UNSUPPORTED && k.acquires == 0);
    db_runtime_set_profile(&rt, &WIDE);

    /* Enabled, no record: waiting, no surface taken. */
    remove(path);
    assert(db_runtime_tick(&rt, t) == DB_RT_WAITING && k.acquires == 0);

    /* Accepted record: acquire once, draw once, UNVERIFIED real data. */
    publish(1, 125);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE);
    assert(k.acquires == 1 && k.draws == 1 && k.owned && k.width == 800 && k.height == 252);
    assert(has_text(&k.last, "12.5 C") && has_text(&k.last, "UNVERIFIED"));
    /* Unchanged data: no redraw (dirty-only drawing). */
    t += 250; wall += 250;
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.draws == 1);
    /* Changed data: one redraw, same ownership. */
    publish(1, 130);
    t += 250;
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.draws == 2 && k.acquires == 1);
    assert(has_text(&k.last, "13.0 C"));

    /* Layout change reconciles: Extended frame drawn without re-acquire. */
    c = cfg(1, DB_EXTENDED); db_runtime_apply_config(&rt, &c);
    db_runtime_apply_config(&rt, &c);              /* idempotent */
    t += 250;
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.draws == 3 && k.acquires == 1);
    assert(has_text(&k.last, "TIME TO DESTINATION"));

    /* Writer stops: after 2 s the record is stale -> clear + release (OEM back). */
    t += DB_VEHICLE_WRITER_TIMEOUT_MS + 1; wall += DB_VEHICLE_WRITER_TIMEOUT_MS + 1;
    assert(db_runtime_tick(&rt, t) == DB_RT_WAITING);
    assert(!k.owned && !k.visible && k.clears == 1 && k.releases == 1);
    /* Repeated waiting ticks do nothing to the backend. */
    t += 250;
    assert(db_runtime_tick(&rt, t) == DB_RT_WAITING && k.releases == 1);

    /* Writer back (new session after HMI restart): re-acquire and draw. */
    publish(2, 125);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.acquires == 2 && k.draws == 4);

    /* Profile change: surface released, re-acquired with the new geometry. */
    {
        db_runtime_profile tall = { 640, 360, {0, 0, 640, 360} };
        db_runtime_set_profile(&rt, &tall);
        assert(!k.owned && k.releases == 2);
        assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.width == 640 && k.acquires == 3);
        db_runtime_set_profile(&rt, &tall);        /* identical: no-op */
        assert(k.owned && k.releases == 2);
        db_runtime_set_profile(&rt, &WIDE);
        assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.width == 800);
    }

    /* Disable: immediate clear + release, and later ticks stay silent. */
    draws = k.draws;
    c = cfg(0, DB_EXTENDED); db_runtime_apply_config(&rt, &c);
    assert(!k.owned && !k.visible && rt.state == DB_RT_DISABLED);
    for (i = 0; i < 3; ++i) { t += 250; assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED); }
    assert(k.draws == draws);

    /* Invalid config: disabled default, nothing drawn. */
    c = cfg(1, DB_COMPACT); c.version = 9;
    db_runtime_apply_config(&rt, &c);
    assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED && !k.owned);
    assert(rt.config.enabled == 0 && rt.config.layout == DB_COMPACT);

    /* Acquire failures: bounded exponential backoff, no retry before the deadline. */
    c = cfg(1, DB_COMPACT); db_runtime_apply_config(&rt, &c);
    k.fail_acquire = 100;
    {
        uint64_t expect = DB_RT_BACKOFF_MIN_MS;
        int before;
        for (i = 0; i < 8; ++i) {
            publish(2, 125);
            before = k.acquires;
            assert(db_runtime_tick(&rt, t) == DB_RT_RECOVERING && k.acquires == before + 1);
            assert(rt.backoff_ms == expect && rt.retry_at_ms == t + expect);
            assert(db_runtime_tick(&rt, t + expect - 1) == DB_RT_RECOVERING && k.acquires == before + 1);
            t += expect; wall += expect;
            expect = expect * 2 > DB_RT_BACKOFF_MAX_MS ? DB_RT_BACKOFF_MAX_MS : expect * 2;
        }
        assert(rt.backoff_ms == DB_RT_BACKOFF_MAX_MS);   /* capped */
    }
    k.fail_acquire = 0;
    publish(2, 125);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && rt.backoff_ms == 0);

    /* Draw failure: clear + release (partial draw hidden), then recover. */
    publish(2, 140);
    k.fail_draw = 1;
    t += 250;
    assert(db_runtime_tick(&rt, t) == DB_RT_RECOVERING);
    assert(!k.owned && !k.visible && rt.draw_failures == 1);
    t += DB_RT_BACKOFF_MIN_MS; wall += DB_RT_BACKOFF_MIN_MS;
    publish(2, 140);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && has_text(&k.last, "14.0 C"));

    /* Corrupt record: provider failure -> waiting, output released. */
    {
        FILE *f = fopen(path, "wb"); assert(f); fputs("garbage", f); assert(!fclose(f));
        t += 250;
        assert(db_runtime_tick(&rt, t) == DB_RT_WAITING && !k.owned);
    }

    /* Shutdown stays disabled across ticks with fresh data until explicit enable. */
    publish(2, 125);
    c = cfg(1, DB_EXTENDED); db_runtime_apply_config(&rt, &c);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE);
    db_runtime_shutdown(&rt);
    db_runtime_shutdown(&rt);
    assert(!k.owned && !k.visible && rt.state == DB_RT_DISABLED);
    {
        int acquires = k.acquires, clears = k.clears, releases = k.releases;
        unsigned reads = wall_reads;
        draws = k.draws;
        for (i = 0; i < 3; ++i) {
            t += 250; wall += 250; publish(2, 125);
            assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED);
        }
        assert(k.acquires == acquires && k.draws == draws);
        assert(k.clears == clears && k.releases == releases && wall_reads == reads);
        /* Reapply the pre-shutdown enabled intent to restart, keeping the layout. */
        db_runtime_apply_config(&rt, &c);
        assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE);
        assert(k.acquires == acquires + 1 && k.draws == draws + 1);
        assert(has_text(&k.last, "TIME TO DESTINATION"));
    }
    /* Shutdown during recovery cancels retry; explicit enable may retry immediately. */
    k.fail_draw = 1; publish(2, 140);
    assert(db_runtime_tick(&rt, t) == DB_RT_RECOVERING && rt.retry_at_ms > t);
    db_runtime_shutdown(&rt);
    assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED);
    assert(rt.backoff_ms == 0 && rt.retry_at_ms == 0);
    db_runtime_apply_config(&rt, &c);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && has_text(&k.last, "14.0 C"));
    db_runtime_shutdown(&rt);
    assert(k.granted == k.releases);                  /* every granted surface was released */

    /* Every incomplete callback combination is rejected before any backend call. */
    for (i = 0; i < 15; ++i) {
        fake partial;
        db_output_backend incomplete;
        db_runtime r2;
        memset(&partial, 0, sizeof(partial));
        incomplete.context = &partial;
        incomplete.acquire = i & 1 ? f_acquire : NULL;
        incomplete.draw = i & 2 ? f_draw : NULL;
        incomplete.clear = i & 4 ? f_clear : NULL;
        incomplete.release = i & 8 ? f_release : NULL;
        db_runtime_init(&r2, &incomplete, path, wall_cb);
        db_runtime_set_profile(&r2, &WIDE);
        c = cfg(1, DB_COMPACT); db_runtime_apply_config(&r2, &c);
        publish(3, 125);
        assert(db_runtime_tick(&r2, t) == DB_RT_RECOVERING);
        assert(!partial.acquires && !partial.draws);
        assert(db_runtime_tick(&r2, t + DB_RT_BACKOFF_MIN_MS - 1) == DB_RT_RECOVERING);
        assert(db_runtime_tick(&r2, t + DB_RT_BACKOFF_MIN_MS) == DB_RT_RECOVERING);
        c.enabled = 0; db_runtime_apply_config(&r2, &c);
        db_runtime_shutdown(&r2);
        assert(db_runtime_tick(&r2, t) == DB_RT_DISABLED);
        assert(!partial.owned && !partial.visible && !partial.acquires && !partial.draws);
        assert(!partial.clears && !partial.releases);
    }
    assert(db_runtime_tick(NULL, 0) == DB_RT_DISABLED);
    db_runtime_init(NULL, &be, NULL, NULL); db_runtime_shutdown(NULL);
    assert(!strcmp(db_runtime_state_name(DB_RT_ACTIVE), "active"));
    assert(!strcmp(db_runtime_state_name((db_runtime_state)99), "unknown"));
    remove(path);
    puts("dashboard_runtime_test: states, data-before-ownership, dirty redraw, reconcile, stale writer release, bounded backoff, draw failure, latched shutdown/restart, all incomplete backends rejected PASS");
    return 0;
}
