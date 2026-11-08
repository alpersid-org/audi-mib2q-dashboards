/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-C07 tests: atomic settings store and live settings in the runtime
 * (persisted intent vs applied state, disable latency, reboot/profile gate,
 * shutdown latch). Host only; fake backend; real DBV1 records. */
#include "../dashboard/runtime.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define W UINT64_C(1700000000000)
static char rec[512], cfgp[512], dir[512];
static uint64_t wall;
static uint32_t seq;

static void put32(unsigned char *b, uint32_t v)
{ b[0] = (unsigned char)(v >> 24); b[1] = (unsigned char)(v >> 16); b[2] = (unsigned char)(v >> 8); b[3] = (unsigned char)v; }
static void put64(unsigned char *b, uint64_t v) { put32(b, (uint32_t)(v >> 32)); put32(b + 4, (uint32_t)v); }
static void publish(void)
{
    unsigned char b[DB_VEHICLE_RECORD_SIZE];
    uint32_t h = 0x811c9dc5u;
    int i;
    FILE *f;
    memset(b, 0, sizeof(b));
    put32(b, 0x44425631u); put32(b + 4, ++seq); put64(b + 8, wall);
    put32(b + 16, 125); put32(b + 20, 620000); put32(b + 24, 42500); put32(b + 28, 2280);
    put64(b + 32, wall); put64(b + 40, wall); put64(b + 48, wall);
    b[56] = 2; put32(b + 60, 5); memcpy(b + 64, "City Centre", 11);
    for (i = 0; i < DB_VEHICLE_RECORD_SIZE - 4; ++i) h = (h ^ b[i]) * 16777619u;
    put32(b + DB_VEHICLE_RECORD_SIZE - 4, h);
    f = fopen(rec, "wb"); assert(f); assert(fwrite(b, 1, sizeof(b), f) == sizeof(b)); assert(!fclose(f));
}
static uint64_t wall_cb(void) { return wall; }

static size_t slurp(const char *p, char *buf, size_t cap)
{
    FILE *f = fopen(p, "rb");
    size_t n;
    if (!f) return 0;
    n = fread(buf, 1, cap, f);
    fclose(f);
    return n;
}

static int leftover_temp(void)
{
    char t[600];
    snprintf(t, sizeof(t), "%s.tmp.", cfgp);
    /* store uses <path>.tmp.<pid>; check the pid-specific name is gone */
    {
        char exact[640];
        snprintf(exact, sizeof(exact), "%s%ld", t, (long)getpid());
        return access(exact, F_OK) == 0;
    }
}

static db_config cfg(int enabled, db_layout layout)
{ db_config c; db_config_default(&c); c.enabled = enabled; c.layout = layout; return c; }

static void store(void)
{
    char buf[1024], before[1024];
    size_t n, nb;
    db_config c = cfg(1, DB_EXTENDED), back, bad;
    remove(cfgp);
    assert(db_config_store_path(cfgp, &c) && !leftover_temp());
    n = slurp(cfgp, buf, sizeof(buf));
    assert(db_config_parse(buf, n, &back) && back.enabled == 1 && back.layout == DB_EXTENDED);
    assert(db_config_load_path(cfgp, &back) == DB_CONFIG_LOADED && back.enabled);
    /* Invalid config: refused, old file byte-identical, no temp left. */
    nb = slurp(cfgp, before, sizeof(before));
    bad = c; bad.version = 7;
    assert(!db_config_store_path(cfgp, &bad) && !leftover_temp());
    n = slurp(cfgp, buf, sizeof(buf));
    assert(n == nb && !memcmp(buf, before, n));
    assert(!db_config_store_path(NULL, &c) && !db_config_store_path(cfgp, NULL));
    /* Directory that does not exist: fails, nothing created. */
    {
        char missing[700];
        snprintf(missing, sizeof(missing), "%s/no-such-dir/x.cfg", dir);
        assert(!db_config_store_path(missing, &c));
    }
    /* Overwrite with a different config replaces atomically. */
    c = cfg(0, DB_COMPACT);
    assert(db_config_store_path(cfgp, &c));
    assert(db_config_load_path(cfgp, &back) == DB_CONFIG_LOADED && !back.enabled && back.layout == DB_COMPACT);
}

typedef struct { int owned, visible, acquires, draws, releases; db_frame last; } fake;
static int has_text(const db_frame *f, const char *s)
{
    size_t i;
    for (i = 0; i < f->count; ++i)
        if (f->commands[i].kind == DB_DRAW_TEXT && !strcmp(f->commands[i].text, s)) return 1;
    return 0;
}
static int f_acq(void *c, int w, int h, cr_rect_t a) { fake *k = c; (void)w; (void)h; (void)a; assert(!k->owned); k->owned = 1; k->acquires++; return 1; }
static int f_draw(void *c, const db_frame *f) { fake *k = c; assert(k->owned); k->draws++; k->visible = 1; k->last = *f; return 1; }
static void f_clear(void *c) { fake *k = c; k->visible = 0; }
static void f_rel(void *c) { fake *k = c; assert(!k->visible); k->owned = 0; k->releases++; }

static void live(void)
{
    static const db_runtime_profile WIDE = { 800, 252, {0, 0, 800, 252} };
    fake k;
    db_output_backend be;
    db_runtime rt;
    db_config c, p;
    db_signal_policy reg;
    uint64_t t = 100000;
    FILE *f;
    memset(&k, 0, sizeof(k));
    be.context = &k; be.acquire = f_acq; be.draw = f_draw; be.clear = f_clear; be.release = f_rel;
    wall = W;
    db_runtime_init(&rt, &be, rec, wall_cb);
    db_signal_policy_default(&reg);
    db_runtime_set_signal_policy(&rt, &reg, DB_ELIGIBILITY_VALIDATION);
    db_runtime_set_profile(&rt, &WIDE);
    publish();

    /* Missing file: persisted MISSING, applied disabled default, no output. */
    remove(cfgp);
    assert(db_runtime_load_settings(&rt, cfgp) == DB_CONFIG_MISSING);
    assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED && !k.owned);

    /* Persisted enabled Extended -> applied and drawn on the next cycle. */
    c = cfg(1, DB_EXTENDED); assert(db_config_store_path(cfgp, &c));
    assert(db_runtime_load_settings(&rt, cfgp) == DB_CONFIG_LOADED);
    assert(db_runtime_applied_config(&rt)->enabled && db_runtime_applied_config(&rt)->layout == DB_EXTENDED);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && has_text(&k.last, "TIME TO DESTINATION"));
    /* Reloading the same file every cycle is a no-op (no redraw, no re-acquire). */
    assert(db_runtime_load_settings(&rt, cfgp) == DB_CONFIG_LOADED);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.draws == 1 && k.acquires == 1);

    /* Layout change via the file: redraw on the next cycle, same surface. */
    c = cfg(1, DB_COMPACT); assert(db_config_store_path(cfgp, &c));
    db_runtime_load_settings(&rt, cfgp);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.acquires == 1 && k.draws == 2);
    assert(!has_text(&k.last, "TIME TO DESTINATION"));

    /* Disable via the file: output released by the reload itself, before any tick
     * (latency: within the same controller cycle, no reconnect). */
    c = cfg(0, DB_COMPACT); assert(db_config_store_path(cfgp, &c));
    db_runtime_load_settings(&rt, cfgp);
    assert(!k.owned && !k.visible && k.releases == 1);
    assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED);

    /* Invalid file (unknown version): fails closed; persisted intent differs from
     * applied state and is reported as INVALID. */
    f = fopen(cfgp, "wb"); assert(f); fputs("version=2\nenabled=1\n", f); assert(!fclose(f));
    assert(db_runtime_load_settings(&rt, cfgp) == DB_CONFIG_INVALID);
    assert(db_runtime_persisted_settings(&rt, &p) == DB_CONFIG_INVALID && !p.enabled);
    assert(!db_runtime_applied_config(&rt)->enabled);
    assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED && !k.owned);
    assert(db_manager_get_status(&rt.manager) == DB_MANAGER_OFF_CONFIG);
    /* Partial file (truncated line) also fails closed. */
    f = fopen(cfgp, "wb"); assert(f); fputs("version=1\nenab", f); assert(!fclose(f));
    assert(db_runtime_load_settings(&rt, cfgp) == DB_CONFIG_INVALID && !db_runtime_applied_config(&rt)->enabled);

    /* "Reboot": a fresh runtime with an old enabled file but no supported profile
     * must not draw; the file cannot bypass the profile gate. */
    {
        db_runtime r2;
        fake k2;
        db_output_backend b2 = be;
        memset(&k2, 0, sizeof(k2)); b2.context = &k2;
        c = cfg(1, DB_EXTENDED); assert(db_config_store_path(cfgp, &c));
        db_runtime_init(&r2, &b2, rec, wall_cb);
        db_runtime_set_signal_policy(&r2, &reg, DB_ELIGIBILITY_VALIDATION);
        assert(db_runtime_load_settings(&r2, cfgp) == DB_CONFIG_LOADED);
        assert(db_runtime_tick(&r2, t) == DB_RT_UNSUPPORTED && !k2.acquires);
        {
            db_runtime_profile bad = { 210, 153, {0, 0, 210, 153} };
            db_runtime_set_profile(&r2, &bad);
            assert(db_runtime_tick(&r2, t) == DB_RT_UNSUPPORTED && !k2.acquires);
        }
        /* Production mode (default register) also draws nothing after reboot. */
        db_runtime_set_profile(&r2, &WIDE);
        db_runtime_set_signal_policy(&r2, &reg, DB_ELIGIBILITY_PRODUCTION);
        assert(db_runtime_tick(&r2, t) == DB_RT_WAITING && !k2.acquires);
        db_runtime_shutdown(&r2);
    }

    /* Shutdown latch: per-cycle reloads of an enabled file never restart output. */
    db_runtime_load_settings(&rt, cfgp);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE);
    db_runtime_shutdown(&rt);
    assert(db_runtime_load_settings(&rt, cfgp) == DB_CONFIG_LOADED);
    assert(db_runtime_persisted_settings(&rt, &p) == DB_CONFIG_LOADED && p.enabled);
    assert(!db_runtime_applied_config(&rt)->enabled);           /* intent != applied */
    assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED && !k.owned);
    /* Explicit apply ends the shutdown. */
    c = cfg(1, DB_EXTENDED); db_runtime_apply_config(&rt, &c);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE);
    db_runtime_shutdown(&rt);
    assert(db_runtime_load_settings(NULL, cfgp) == DB_CONFIG_INVALID);
    assert(db_runtime_applied_config(NULL) == NULL);
    assert(db_runtime_persisted_settings(NULL, &p) == DB_CONFIG_INVALID && !p.enabled);
    remove(rec); remove(cfgp);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    snprintf(dir, sizeof(dir), "%s", argv[1]);
    snprintf(rec, sizeof(rec), "%s/carplay_dashboard_settings", argv[1]);
    snprintf(cfgp, sizeof(cfgp), "%s/carplay_dashboard_settings.cfg", argv[1]);
    store();
    live();
    puts("dashboard_settings_test: atomic store (canonical, invalid refused, old file kept, no temp), live reload persisted vs applied, layout/disable without reconnect, fail-closed invalid/partial, reboot profile gate, shutdown latch PASS");
    return 0;
}
