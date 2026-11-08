/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-C04 per-signal eligibility filter; see signal_policy.h. */
#include "signal_policy.h"
#include <string.h>

void db_signal_policy_default(db_signal_policy *p)
{
    if (!p) return;
    p->state[DB_SIG_TEMPERATURE] = DB_SIGNAL_CANDIDATE; /* DSICarKombi, VehicleDataModule */
    p->state[DB_SIG_RANGE] = DB_SIGNAL_CANDIDATE;       /* total vs range-1 unverified */
    p->state[DB_SIG_MEDIA] = DB_SIGNAL_OMITTED;         /* no AA source in the repo */
    p->state[DB_SIG_NAVIGATION] = DB_SIGNAL_CANDIDATE;  /* AA trip feed, rich-nav apps */
}

int db_signal_policy_valid(const db_signal_policy *p)
{
    int i;
    if (!p) return 0;
    for (i = 0; i < DB_SIG_COUNT; ++i)
        if (p->state[i] != DB_SIGNAL_OMITTED && p->state[i] != DB_SIGNAL_CANDIDATE &&
            p->state[i] != DB_SIGNAL_ACCEPTED) return 0;
    return 1;
}

static void *group(db_snapshot *s, db_signal sig, size_t *size)
{
    switch (sig) {
    case DB_SIG_TEMPERATURE: *size = sizeof(s->temperature); return &s->temperature;
    case DB_SIG_RANGE: *size = sizeof(s->range); return &s->range;
    case DB_SIG_MEDIA: *size = sizeof(s->media); return &s->media;
    case DB_SIG_NAVIGATION: *size = sizeof(s->navigation); return &s->navigation;
    default: *size = 0; return NULL;
    }
}

/* Every group starts with its db_sample. */
static db_sample *sample(db_snapshot *s, db_signal sig)
{
    size_t n;
    return (db_sample *)group(s, sig, &n);
}

int db_signal_policy_apply(const db_signal_policy *p, db_eligibility_mode mode, db_snapshot *s)
{
    int i, kept = 0, ok;
    if (!s) return 0;
    ok = db_signal_policy_valid(p) &&
         (mode == DB_ELIGIBILITY_PRODUCTION || mode == DB_ELIGIBILITY_VALIDATION);
    for (i = 0; i < DB_SIG_COUNT; ++i) {
        size_t n;
        void *g = group(s, (db_signal)i, &n);
        db_sample *m = sample(s, (db_signal)i);
        db_source src = m->source == DB_SOURCE_VERIFIED ? DB_SOURCE_VEHICLE : m->source;
        int keep = ok && m->availability != DB_MISSING && src == DB_SOURCE_VEHICLE &&
                   (p->state[i] == DB_SIGNAL_ACCEPTED ||
                    (p->state[i] == DB_SIGNAL_CANDIDATE && mode == DB_ELIGIBILITY_VALIDATION));
        if (!keep) { memset(g, 0, n); continue; }
        m->source = p->state[i] == DB_SIGNAL_ACCEPTED ? DB_SOURCE_VERIFIED : DB_SOURCE_VEHICLE;
        ++kept;
    }
    return kept;
}

const char *db_signal_name(db_signal sig)
{
    switch (sig) {
    case DB_SIG_TEMPERATURE: return "temperature";
    case DB_SIG_RANGE: return "range";
    case DB_SIG_MEDIA: return "media";
    case DB_SIG_NAVIGATION: return "navigation";
    default: return "unknown";
    }
}
