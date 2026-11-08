/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-C04 tests: per-signal production eligibility (signal_policy.c) and its use by
 * the runtime. No signal is ACCEPTED in the compiled register. */
#include "../dashboard/runtime.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static db_snapshot vehicle(uint64_t now)
{
    db_snapshot s;
    assert(db_mock_read(NULL, now, &s));   /* fixture values only; source relabelled */
    s.temperature.sample.source = s.range.sample.source = DB_SOURCE_VEHICLE;
    s.media.sample.source = s.navigation.sample.source = DB_SOURCE_VEHICLE;
    return s;
}

static int zero_group(const void *g, size_t n)
{
    const unsigned char *b = g;
    size_t i;
    for (i = 0; i < n; ++i) if (b[i]) return 0;
    return 1;
}

static void policy_unit(void)
{
    db_signal_policy reg, p;
    db_snapshot s;
    db_widget_view v;
    db_signal_policy_default(&reg);
    assert(db_signal_policy_valid(&reg));
    assert(reg.state[DB_SIG_TEMPERATURE] == DB_SIGNAL_CANDIDATE);
    assert(reg.state[DB_SIG_RANGE] == DB_SIGNAL_CANDIDATE);
    assert(reg.state[DB_SIG_NAVIGATION] == DB_SIGNAL_CANDIDATE);
    assert(reg.state[DB_SIG_MEDIA] == DB_SIGNAL_OMITTED);

    /* PRODUCTION with the register: nothing accepted -> everything removed, zeroed. */
    s = vehicle(100);
    assert(db_signal_policy_apply(&reg, DB_ELIGIBILITY_PRODUCTION, &s) == 0);
    assert(zero_group(&s.temperature, sizeof(s.temperature)) && zero_group(&s.range, sizeof(s.range)));
    assert(zero_group(&s.navigation, sizeof(s.navigation)) && zero_group(&s.media, sizeof(s.media)));

    /* VALIDATION: candidates kept as VEHICLE (UNVERIFIED); omitted media removed. */
    s = vehicle(100);
    assert(db_signal_policy_apply(&reg, DB_ELIGIBILITY_VALIDATION, &s) == 3);
    assert(s.temperature.sample.source == DB_SOURCE_VEHICLE && s.temperature.decicelsius == 125);
    assert(s.navigation.sample.source == DB_SOURCE_VEHICLE && s.navigation.remaining_meters == 42500);
    assert(zero_group(&s.media, sizeof(s.media)));
    db_widget_format(DB_TEMPERATURE, &s, 100, &v);
    assert(!strcmp(v.badge, "UNVERIFIED"));

    /* MOCK never passes, in either mode. */
    assert(db_mock_read(NULL, 100, &s));
    assert(db_signal_policy_apply(&reg, DB_ELIGIBILITY_VALIDATION, &s) == 0);
    assert(db_mock_read(NULL, 100, &s));
    p = reg; p.state[DB_SIG_TEMPERATURE] = DB_SIGNAL_ACCEPTED;
    assert(db_signal_policy_apply(&p, DB_ELIGIBILITY_PRODUCTION, &s) == 0);

    /* Per-signal promotion: only the accepted signal becomes VERIFIED. */
    s = vehicle(100);
    assert(db_signal_policy_apply(&p, DB_ELIGIBILITY_PRODUCTION, &s) == 1);
    assert(s.temperature.sample.source == DB_SOURCE_VERIFIED);
    assert(zero_group(&s.range, sizeof(s.range)) && zero_group(&s.navigation, sizeof(s.navigation)));
    db_widget_format(DB_TEMPERATURE, &s, 100, &v);
    assert(v.status == DB_VALUE_OK && !strcmp(v.badge, "VERIFIED"));
    s = vehicle(100);
    assert(db_signal_policy_apply(&p, DB_ELIGIBILITY_VALIDATION, &s) == 3);
    assert(s.temperature.sample.source == DB_SOURCE_VERIFIED);
    assert(s.range.sample.source == DB_SOURCE_VEHICLE);    /* still a candidate */

    /* A provider cannot assert VERIFIED: treated as VEHICLE. */
    s = vehicle(100); s.range.sample.source = DB_SOURCE_VERIFIED;
    assert(db_signal_policy_apply(&reg, DB_ELIGIBILITY_PRODUCTION, &s) == 0);
    s = vehicle(100); s.range.sample.source = DB_SOURCE_VERIFIED;
    assert(db_signal_policy_apply(&reg, DB_ELIGIBILITY_VALIDATION, &s) == 3);
    assert(s.range.sample.source == DB_SOURCE_VEHICLE);
    /* UNKNOWN and missing groups are not counted; invalid samples stay (shown INVALID). */
    s = vehicle(100); s.temperature.sample.source = DB_SOURCE_UNKNOWN;
    s.range.sample.availability = DB_MISSING; s.navigation.sample.availability = DB_INVALID;
    assert(db_signal_policy_apply(&reg, DB_ELIGIBILITY_VALIDATION, &s) == 1);
    assert(zero_group(&s.temperature, sizeof(s.temperature)) && zero_group(&s.range, sizeof(s.range)));
    assert(s.navigation.sample.availability == DB_INVALID);
    /* An omitted signal is removed even when the source is fine. */
    p = reg; p.state[DB_SIG_NAVIGATION] = DB_SIGNAL_OMITTED;
    s = vehicle(100);
    assert(db_signal_policy_apply(&p, DB_ELIGIBILITY_VALIDATION, &s) == 2);
    assert(zero_group(&s.navigation, sizeof(s.navigation)));

    /* Invalid policy / mode / NULL: everything removed. */
    p = reg; p.state[DB_SIG_RANGE] = (db_signal_state)7;
    assert(!db_signal_policy_valid(&p) && !db_signal_policy_valid(NULL));
    s = vehicle(100); assert(db_signal_policy_apply(&p, DB_ELIGIBILITY_VALIDATION, &s) == 0);
    s = vehicle(100); assert(db_signal_policy_apply(NULL, DB_ELIGIBILITY_VALIDATION, &s) == 0);
    s = vehicle(100); assert(db_signal_policy_apply(&reg, (db_eligibility_mode)5, &s) == 0);
    assert(zero_group(&s.temperature, sizeof(s.temperature)));
    assert(db_signal_policy_apply(&reg, DB_ELIGIBILITY_VALIDATION, NULL) == 0);
    db_signal_policy_default(NULL);
    assert(!strcmp(db_signal_name(DB_SIG_NAVIGATION), "navigation"));
    assert(!strcmp(db_signal_name((db_signal)9), "unknown"));
}

/* ---- runtime: production shows accepted signals only, VERIFIED per signal ---- */
#define W UINT64_C(1700000000000)
static char path[512];
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
    b[56] = 2; put32(b + 60, 7); memcpy(b + 64, "City Centre", 11);
    for (i = 0; i < DB_VEHICLE_RECORD_SIZE - 4; ++i) h = (h ^ b[i]) * 16777619u;
    put32(b + DB_VEHICLE_RECORD_SIZE - 4, h);
    f = fopen(path, "wb"); assert(f); assert(fwrite(b, 1, sizeof(b), f) == sizeof(b)); assert(!fclose(f));
}
static uint64_t wall_cb(void) { return wall; }

typedef struct { int owned, visible, draws, releases; db_frame last; } fake;
static int has_text(const db_frame *f, const char *s)
{
    size_t i;
    for (i = 0; i < f->count; ++i)
        if (f->commands[i].kind == DB_DRAW_TEXT && !strcmp(f->commands[i].text, s)) return 1;
    return 0;
}
static int f_acquire(void *c, int w, int h, cr_rect_t a) { fake *k = c; (void)w; (void)h; (void)a; assert(!k->owned); k->owned = 1; return 1; }
static int f_draw(void *c, const db_frame *f) { fake *k = c; assert(k->owned); k->draws++; k->visible = 1; k->last = *f; return 1; }
static void f_clear(void *c) { fake *k = c; k->visible = 0; }
static void f_release(void *c) { fake *k = c; assert(!k->visible); k->owned = 0; k->releases++; }

static void runtime_eligibility(const char *dir)
{
    static const db_runtime_profile WIDE = { 800, 252, {0, 0, 800, 252} };
    fake k;
    db_output_backend be;
    db_runtime rt;
    db_config c;
    db_signal_policy reg, p;
    uint64_t t = 100000;
    snprintf(path, sizeof(path), "%s/carplay_dashboard_policy", dir);
    memset(&k, 0, sizeof(k));
    be.context = &k; be.acquire = f_acquire; be.draw = f_draw; be.clear = f_clear; be.release = f_release;
    wall = W;
    db_runtime_init(&rt, &be, path, wall_cb);
    db_runtime_set_profile(&rt, &WIDE);
    db_config_default(&c); c.enabled = 1;
    db_runtime_apply_config(&rt, &c);
    publish();
    assert(db_runtime_tick(&rt, t) == DB_RT_WAITING && !k.owned);   /* nothing accepted */

    /* Accept temperature only: production shows exactly that card, VERIFIED. */
    db_signal_policy_default(&reg);
    p = reg; p.state[DB_SIG_TEMPERATURE] = DB_SIGNAL_ACCEPTED;
    db_runtime_set_signal_policy(&rt, &p, DB_ELIGIBILITY_PRODUCTION);
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE && k.draws == 1);
    assert(has_text(&k.last, "12.5 C") && has_text(&k.last, "VERIFIED"));
    assert(!has_text(&k.last, "UNVERIFIED") && !has_text(&k.last, "620 km"));
    assert(!has_text(&k.last, "42.5 km / 38 min") && !has_text(&k.last, "MOCK"));
    db_runtime_set_signal_policy(&rt, &p, DB_ELIGIBILITY_PRODUCTION);  /* identical: no-op */
    assert(k.owned);

    /* Validation adds candidates (UNVERIFIED) alongside the verified signal. */
    db_runtime_set_signal_policy(&rt, &p, DB_ELIGIBILITY_VALIDATION);
    assert(!k.owned && k.releases == 1);                 /* old frame never kept */
    assert(db_runtime_tick(&rt, t) == DB_RT_ACTIVE);
    assert(has_text(&k.last, "VERIFIED") && has_text(&k.last, "UNVERIFIED") && has_text(&k.last, "620 km"));

    /* Withdrawing acceptance releases immediately; production then shows nothing. */
    db_runtime_set_signal_policy(&rt, &reg, DB_ELIGIBILITY_PRODUCTION);
    assert(!k.owned && rt.state == DB_RT_WAITING);
    assert(db_runtime_tick(&rt, t) == DB_RT_WAITING && !k.owned);

    /* Invalid policy/mode: everything removed. */
    db_runtime_set_signal_policy(&rt, &p, (db_eligibility_mode)9);
    assert(rt.mode == DB_ELIGIBILITY_PRODUCTION && db_runtime_tick(&rt, t) == DB_RT_WAITING);
    db_runtime_set_signal_policy(&rt, NULL, DB_ELIGIBILITY_VALIDATION);
    assert(db_runtime_tick(&rt, t) == DB_RT_WAITING && !k.owned);
    db_runtime_set_signal_policy(NULL, &p, DB_ELIGIBILITY_PRODUCTION);

    /* Shutdown latch is not undone by a policy change. */
    db_runtime_set_signal_policy(&rt, &p, DB_ELIGIBILITY_PRODUCTION);
    db_runtime_shutdown(&rt);
    db_runtime_set_signal_policy(&rt, &reg, DB_ELIGIBILITY_VALIDATION);
    assert(db_runtime_tick(&rt, t) == DB_RT_DISABLED && !k.owned);
    remove(path);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    policy_unit();
    runtime_eligibility(argv[1]);
    puts("dashboard_signal_policy_test: register defaults, production/validation eligibility, per-signal VERIFIED promotion, MOCK/UNKNOWN/provider-VERIFIED rejection, invalid policy, runtime release on withdrawal, shutdown latch PASS");
    return 0;
}
