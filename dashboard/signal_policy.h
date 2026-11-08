/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-C04: per-signal production eligibility. See docs/custom-dashboards/signal-acceptance.md. */
#ifndef CUSTOM_DASHBOARD_SIGNAL_POLICY_H
#define CUSTOM_DASHBOARD_SIGNAL_POLICY_H

#include "dashboard.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Snapshot signal groups. Navigation is one atomic group (distance, time, name). */
typedef enum { DB_SIG_TEMPERATURE, DB_SIG_RANGE, DB_SIG_MEDIA, DB_SIG_NAVIGATION, DB_SIG_COUNT } db_signal;

/* OMITTED: never shown (no source, or rejected).
 * CANDIDATE: a plausible existing source, not accepted on this car; shown only in
 *            VALIDATION mode, badged UNVERIFIED.
 * ACCEPTED:  meaning/units/validity/lifetime accepted with recorded evidence; shown
 *            in PRODUCTION and promoted to VERIFIED for that signal only. */
typedef enum { DB_SIGNAL_OMITTED, DB_SIGNAL_CANDIDATE, DB_SIGNAL_ACCEPTED } db_signal_state;

/* PRODUCTION (default): accepted signals only. VALIDATION: an explicit on-car
 * check mode that also shows candidates as UNVERIFIED. Neither mode passes MOCK. */
typedef enum { DB_ELIGIBILITY_PRODUCTION, DB_ELIGIBILITY_VALIDATION } db_eligibility_mode;

typedef struct { db_signal_state state[DB_SIG_COUNT]; } db_signal_policy;

/* The compiled acceptance register (signal-acceptance.md): temperature, range and
 * navigation CANDIDATE; media OMITTED. No signal is ACCEPTED yet. NULL: no-op. */
void db_signal_policy_default(db_signal_policy *policy);
int db_signal_policy_valid(const db_signal_policy *policy);
/* Filters a snapshot in place, per group:
 * - MOCK/UNKNOWN/VERIFIED-from-provider sources are not trusted: MOCK and UNKNOWN
 *   are removed; a provider never asserts VERIFIED, so it is treated as VEHICLE.
 * - OMITTED removed; CANDIDATE kept only in VALIDATION (source VEHICLE);
 *   ACCEPTED kept, VEHICLE promoted to VERIFIED for that group only.
 * Removal zeroes the whole group (no leftover values). An invalid policy or mode
 * removes everything. Returns the number of groups still available. */
int db_signal_policy_apply(const db_signal_policy *policy, db_eligibility_mode mode,
                           db_snapshot *snapshot);
const char *db_signal_name(db_signal signal);

#ifdef __cplusplus
}
#endif
#endif
