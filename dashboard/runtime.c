/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-C05 runtime controller; see runtime.h and docs/custom-dashboards/runtime.md. */
#include "runtime.h"
#include <string.h>

static void leave(db_runtime *rt, db_runtime_state state)
{
    if (rt->shown && rt->backend.clear) rt->backend.clear(rt->backend.context);
    if (rt->owned && rt->backend.release) rt->backend.release(rt->backend.context);
    rt->shown = rt->owned = 0;
    memset(&rt->last, 0, sizeof(rt->last));
    rt->state = state;
}

static void fail(db_runtime *rt, uint64_t now_ms)
{
    leave(rt, DB_RT_RECOVERING);
    rt->backoff_ms = !rt->backoff_ms ? DB_RT_BACKOFF_MIN_MS
                   : rt->backoff_ms >= DB_RT_BACKOFF_MAX_MS / 2 ? DB_RT_BACKOFF_MAX_MS
                   : rt->backoff_ms * 2;
    rt->retry_at_ms = now_ms + rt->backoff_ms;
}

/* Geometry probe only: layouts omit cards without data (scope req. 8), so the
 * tightest fit is every card present. This local snapshot is discarded after the
 * probe; it never reaches the manager or the backend. */
static int profile_supported(const db_runtime_profile *p)
{
    db_snapshot full;
    db_sample s;
    db_frame probe;
    memset(&full, 0, sizeof(full));
    memset(&s, 0, sizeof(s));
    s.availability = DB_AVAILABLE;
    s.source = DB_SOURCE_MOCK;              /* honest label; never displayed */
    full.temperature.sample = full.range.sample = s;
    full.media.sample = full.navigation.sample = s;
    full.temperature.decicelsius = -999;    /* widest plausible values */
    full.range.meters = 9999000;
    memset(full.media.title, 'W', DB_TEXT_CAP - 1);
    memset(full.media.artist, 'W', DB_TEXT_CAP - 1);
    full.navigation.remaining_meters = 999999000;
    full.navigation.remaining_seconds = 31536000;
    memset(full.navigation.destination, 'W', DB_TEXT_CAP - 1);
    return db_layout_build(DB_COMPACT, &full, 0, p->width, p->height, p->area, &probe) &&
           db_layout_build(DB_EXTENDED, &full, 0, p->width, p->height, p->area, &probe);
}

/* Manager provider: the persistent vehicle reader, then the eligibility filter. */
static int eligible_read(void *context, uint64_t now_ms, db_snapshot *out)
{
    db_runtime *rt = context;
    if (!db_vehicle_read(&rt->source, now_ms, out)) return 0;
    db_signal_policy_apply(&rt->policy, rt->mode, out); /* empty -> manager OFF_DATA */
    return 1;
}

void db_runtime_init(db_runtime *rt, const db_output_backend *backend,
                     const char *record_path, uint64_t (*wall_ms)(void))
{
    db_provider p;
    if (!rt) return;
    memset(rt, 0, sizeof(*rt));
    if (backend) rt->backend = *backend;
    rt->source.path = record_path;
    rt->source.wall_ms = wall_ms;
    db_signal_policy_default(&rt->policy);
    rt->mode = DB_ELIGIBILITY_PRODUCTION;
    p.read = eligible_read;
    p.context = rt;
    db_manager_init(&rt->manager, &p);
    db_config_default(&rt->config);
    db_manager_set_config(&rt->manager, &rt->config);
    rt->state = DB_RT_DISABLED;
}

void db_runtime_set_profile(db_runtime *rt, const db_runtime_profile *profile)
{
    if (!rt) return;
    if (profile && rt->profile_ok && !memcmp(profile, &rt->profile, sizeof(*profile))) return;
    leave(rt, rt->state == DB_RT_ACTIVE ? DB_RT_WAITING : rt->state);
    if (profile) rt->profile = *profile;
    else memset(&rt->profile, 0, sizeof(rt->profile));
    rt->profile_ok = profile && profile_supported(profile);
}

void db_runtime_set_signal_policy(db_runtime *rt, const db_signal_policy *policy,
                                  db_eligibility_mode mode)
{
    db_signal_policy next;
    if (!rt) return;
    if (db_signal_policy_valid(policy)) next = *policy;
    else memset(&next, 0, sizeof(next));                 /* all OMITTED */
    if (mode != DB_ELIGIBILITY_PRODUCTION && mode != DB_ELIGIBILITY_VALIDATION) {
        memset(&next, 0, sizeof(next));
        mode = DB_ELIGIBILITY_PRODUCTION;
    }
    if (!memcmp(&next, &rt->policy, sizeof(next)) && mode == rt->mode) return;
    rt->policy = next;
    rt->mode = mode;
    /* Eligibility may have shrunk: never keep showing a frame built under the old
     * policy. Same latch rule as set_profile (shutdown/disabled stay disabled). */
    leave(rt, rt->state == DB_RT_ACTIVE ? DB_RT_WAITING : rt->state);
}

void db_runtime_apply_config(db_runtime *rt, const db_config *config)
{
    db_config next;
    if (!rt) return;
    if (db_config_valid(config)) next = *config;
    else db_config_default(&next);
    if (!memcmp(&next, &rt->config, sizeof(next)) && db_config_valid(config)) return;
    rt->config = next;
    db_manager_set_config(&rt->manager, db_config_valid(config) ? &next : NULL);
    if (!next.enabled) leave(rt, DB_RT_DISABLED);   /* immediate OEM restoration */
    rt->backoff_ms = 0; rt->retry_at_ms = 0;       /* a new intent retries now */
}

db_runtime_state db_runtime_tick(db_runtime *rt, uint64_t now_ms)
{
    const db_frame *frame;
    if (!rt) return DB_RT_DISABLED;
    if (!rt->config.enabled) { leave(rt, DB_RT_DISABLED); return rt->state; }
    if (!rt->profile_ok) { leave(rt, DB_RT_UNSUPPORTED); return rt->state; }
    if (rt->state == DB_RT_RECOVERING && now_ms < rt->retry_at_ms) return rt->state;

    /* Acquisition first (the only file I/O), then ownership: no surface is taken
     * unless there is an accepted frame to show. */
    if (!db_manager_update(&rt->manager, now_ms, rt->profile.width, rt->profile.height,
                           rt->profile.area)) {
        leave(rt, DB_RT_WAITING);
        return rt->state;
    }
    frame = db_manager_frame(&rt->manager);
    if (!rt->owned) {
        /* Reject incomplete backends before taking a surface: every successful
         * acquire must have both a drawing path and a guaranteed cleanup path. */
        if (!rt->backend.acquire || !rt->backend.draw ||
            !rt->backend.clear || !rt->backend.release ||
            !rt->backend.acquire(rt->backend.context, rt->profile.width, rt->profile.height,
                                 rt->profile.area)) {
            rt->acquire_failures++;
            fail(rt, now_ms);
            return rt->state;
        }
        rt->owned = 1;
        rt->shown = 0;
    }
    if (!rt->shown || memcmp(frame, &rt->last, sizeof(*frame))) {
        if (!rt->backend.draw || !rt->backend.draw(rt->backend.context, frame)) {
            rt->draw_failures++;
            rt->shown = 1;                 /* a partial draw may be visible: clear it */
            fail(rt, now_ms);
            return rt->state;
        }
        rt->last = *frame;
        rt->shown = 1;
    }
    rt->backoff_ms = 0;
    rt->state = DB_RT_ACTIVE;
    return rt->state;
}

void db_runtime_shutdown(db_runtime *rt)
{
    if (!rt) return;
    /* Latch disabled intent so an already-scheduled tick cannot restart output.
     * Applying enabled config explicitly starts the next lifecycle. */
    db_config_disable(&rt->config);
    db_manager_disable(&rt->manager);
    rt->backoff_ms = rt->retry_at_ms = 0;
    leave(rt, DB_RT_DISABLED);
}

const char *db_runtime_state_name(db_runtime_state s)
{
    switch (s) {
    case DB_RT_DISABLED: return "disabled";
    case DB_RT_UNSUPPORTED: return "unsupported";
    case DB_RT_WAITING: return "waiting";
    case DB_RT_ACTIVE: return "active";
    case DB_RT_RECOVERING: return "recovering";
    }
    return "unknown";
}
