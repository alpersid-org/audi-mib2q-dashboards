/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-C05: dashboard runtime controller (host-testable consumer and lifecycle).
 * Drives record -> provider -> manager -> frame -> output backend. The backend is a
 * contract; the only implementation in this repository is the host test fake.
 * No native/QNX backend exists here (dashboard.h still refuses QNX builds). */
#ifndef CUSTOM_DASHBOARD_RUNTIME_H
#define CUSTOM_DASHBOARD_RUNTIME_H

#include "dashboard.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DB_RT_BACKOFF_MIN_MS UINT64_C(250)
#define DB_RT_BACKOFF_MAX_MS UINT64_C(8000)

typedef enum {
    DB_RT_DISABLED,    /* config off (default), invalid config or shut down */
    DB_RT_UNSUPPORTED, /* no target profile, or the layouts reject its geometry */
    DB_RT_WAITING,     /* enabled, but no accepted data (missing/stale/invalid record) */
    DB_RT_ACTIVE,      /* surface owned and the current frame shown */
    DB_RT_RECOVERING   /* acquire/draw failed; retry after a bounded backoff */
} db_runtime_state;

/* Output backend contract. Callbacks run synchronously from db_runtime_tick /
 * apply_* / shutdown and must not block or do file I/O on the draw path.
 * All four callbacks are required; an incomplete backend enters RECOVERING
 * without invoking any of its callbacks or acquiring a surface.
 * - acquire: take the output surface for this geometry; 1 owned, 0 failed/unowned.
 * - draw:    present a valid frame on the owned surface; 1 shown, 0 failed.
 * - clear:   show nothing from the dashboard (keep ownership).
 * - release: give the surface back so OEM composition is restored. Idempotent.
 * The runtime calls draw only while owned, and always clear before release when
 * something was shown. */
typedef struct {
    void *context;
    int (*acquire)(void *context, int width, int height, cr_rect_t area);
    int (*draw)(void *context, const db_frame *frame);
    void (*clear)(void *context);
    void (*release)(void *context);
} db_output_backend;

/* Explicit target geometry (source pixels). A host test input, never a claim that
 * a vehicle surface has these dimensions. */
typedef struct { int width, height; cr_rect_t area; } db_runtime_profile;

/* Caller-owned; must not be moved after db_runtime_init (the manager's provider
 * points at the embedded reader). Single-threaded. Fields are private. */
typedef struct {
    db_output_backend backend;
    db_runtime_profile profile;
    int profile_ok;
    db_vehicle_source source;   /* one persistent, stateful reader */
    db_manager manager;
    db_config config;           /* last applied (valid) configuration */
    db_runtime_state state;
    int owned, shown;
    db_frame last;              /* last frame passed to draw (change detection) */
    uint64_t retry_at_ms, backoff_ms;
    unsigned acquire_failures, draw_failures;
} db_runtime;

/* Starts DISABLED with the default config and no profile. record_path/wall_ms
 * configure the DBV1 reader (NULL = production defaults). The data source is
 * always the vehicle record reader: there is no mock path in the runtime. */
void db_runtime_init(db_runtime *rt, const db_output_backend *backend,
                     const char *record_path, uint64_t (*wall_ms)(void));
/* NULL or geometry the layouts reject -> UNSUPPORTED at the next tick. A changed
 * profile releases the surface first (it must be re-acquired for new geometry). */
void db_runtime_set_profile(db_runtime *rt, const db_runtime_profile *profile);
/* Idempotent reconciler: an identical config is a no-op. NULL/invalid applies the
 * disabled default. Disabling clears and releases output immediately. */
void db_runtime_apply_config(db_runtime *rt, const db_config *config);
/* One controller step at monotonic now_ms (call periodically, e.g. every 250 ms):
 * read the record once (I/O happens here, never in draw), then converge output. */
db_runtime_state db_runtime_tick(db_runtime *rt, uint64_t now_ms);
/* Clear and release, disable runtime/manager config, and cancel pending retries.
 * Repeated calls and later ticks stay DISABLED without reads or backend calls.
 * Restart by explicitly applying enabled config; the selected layout is kept. */
void db_runtime_shutdown(db_runtime *rt);
const char *db_runtime_state_name(db_runtime_state state);

#ifdef __cplusplus
}
#endif
#endif
