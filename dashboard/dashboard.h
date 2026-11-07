/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CUSTOM_DASHBOARD_H
#define CUSTOM_DASHBOARD_H

#ifdef __QNXNTO__
#error "Host-only prototype: no validated vehicle dashboard surface"
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "../maneuver_render/visible_area.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DB_TEXT_CAP 96
#define DB_COMMAND_CAP 48
#define DB_MAX_AGE_MS UINT64_C(30000)

typedef enum { DB_MISSING, DB_AVAILABLE, DB_INVALID } db_availability;
/* VERIFIED is reserved for a source validated on this car (PLAN step 10); none ships.
 * VEHICLE (P2-08): live data from an existing MMI pipeline, not yet validated on this
 * car; widgets badge it UNVERIFIED. MOCK is for automated tests/development only. */
typedef enum { DB_SOURCE_UNKNOWN, DB_SOURCE_MOCK, DB_SOURCE_VERIFIED, DB_SOURCE_VEHICLE } db_source;
typedef struct {
    db_availability availability;
    db_source source;
    uint64_t sampled_ms; /* same monotonic clock/epoch as caller's now_ms; 0 valid */
} db_sample;

typedef struct { db_sample sample; int32_t decicelsius; } db_temperature;
typedef struct { db_sample sample; int32_t meters; } db_distance;
/* P2-07 (scope req. 8): trip distance and average consumption were removed. No
 * existing pipeline supplies them; re-add only with a verified source. */
typedef struct {
    db_sample sample;
    char title[DB_TEXT_CAP], artist[DB_TEXT_CAP];
} db_media;
/* Atomic group: all three fields must describe the same destination/estimate.
 * Future adapters must use the OLDEST contributing sample time, never refresh
 * old estimates just because a new destination string arrived.
 * remaining_meters/remaining_seconds may be -1 = not supplied by the source
 * (matches aa_hook/aa_trip.h); an empty destination means not supplied. */
typedef struct {
    db_sample sample;
    int32_t remaining_meters, remaining_seconds;
    char destination[DB_TEXT_CAP];
} db_navigation;
typedef struct {
    db_temperature temperature;
    db_distance range;
    db_media media;
    db_navigation navigation;
} db_snapshot;

/* Callback writes an owned, bounded snapshot; return exactly 1 on success.
 * Synchronous, no retained output pointer. Caller/provider manage synchronization.
 * This is an in-process interface, not a wire ABI or an untrusted-code sandbox. */
typedef int (*db_read_fn)(void *context, uint64_t now_ms, db_snapshot *out);
typedef struct { db_read_fn read; void *context; } db_provider;
/* Zero-initializes callback output; on any failure clears old caller data.
 * Returns 1 success, 0 failure. NULL arguments fail safely. */
int db_snapshot_read(const db_provider *provider, uint64_t now_ms, db_snapshot *out);
int db_mock_read(void *context, uint64_t now_ms, db_snapshot *out);

/* ---- P2-08: VehicleDataProvider (reader of the HMI DashboardModule record) ---- */
#define DB_VEHICLE_RECORD_SIZE 256
#define DB_VEHICLE_PATH "/tmp/carplay_dashboard"
/* The writer publishes every 500 ms; a record whose (session, sequence) has not
 * advanced for this long means the writer stopped (review: session invalidation). */
#define DB_VEHICLE_WRITER_TIMEOUT_MS UINT64_C(2000)
/* Context for db_vehicle_read. path/wall_ms are configuration: wall_ms returns the
 * system clock in ms (same clock as Java currentTimeMillis); NULL path/wall_ms use
 * DB_VEHICLE_PATH / CLOCK_REALTIME. The remaining fields are reader state; zero-
 * initialize the struct and keep it for the reader's lifetime. Not thread-safe. */
typedef struct {
    const char *path;
    uint64_t (*wall_ms)(void);
    uint32_t last_session, last_sequence; /* last accepted record; session 0 = none */
    uint64_t last_advance_wall;           /* wall time the sequence last advanced */
} db_vehicle_source;
/* Validates a DBV1 record (magic, checksum, zero padding, non-zero session and
 * sequence, bounds, record age) and maps each field's system-clock sample time to an
 * age on the caller's now_ms clock. Stateless: session/sequence tracking is done by
 * db_vehicle_read.
 * Fields: temperature, range, navigation group (AA only); media is never supplied.
 * Returns 1 and fills out (source DB_SOURCE_VEHICLE), or 0 for any invalid/stale
 * record. Record older than DB_MAX_AGE_MS or written in the future fails. */
int db_vehicle_parse(const unsigned char *record, size_t len, uint64_t wall_now_ms,
                     uint64_t now_ms, db_snapshot *out);
/* db_read_fn: bounded read of the record file; wall clock sampled AFTER the read
 * (review R3); db_vehicle_parse; then writer-liveness checks against the context's
 * state: a new session is accepted and tracked; within one session the sequence must
 * not go backwards, and an unchanged sequence older than
 * DB_VEHICLE_WRITER_TIMEOUT_MS fails. context must be a db_vehicle_source (or NULL
 * for a stateless read with defaults and no liveness tracking). */
int db_vehicle_read(void *context, uint64_t now_ms, db_snapshot *out);

/* Only widgets backed by an existing vehicle/MMI pipeline (scope req. 8):
 * temperature/range (VehicleDataModule, DSICarKombi), media (now-playing DSI),
 * navigation group (AA trip feed / CarPlay RGD). NAVIGATION is the combined
 * Compact card; NAV_DISTANCE, NAV_TIME and DESTINATION split the same atomic
 * navigation sample for Extended. No speed, warning or control widgets. */
typedef enum {
    DB_TEMPERATURE, DB_RANGE, DB_MEDIA, DB_NAVIGATION,
    DB_NAV_DISTANCE, DB_NAV_TIME, DB_DESTINATION,
    DB_WIDGET_COUNT
} db_widget;
typedef enum {
    DB_VALUE_MISSING, DB_VALUE_INVALID, DB_VALUE_FUTURE, DB_VALUE_STALE, DB_VALUE_OK
} db_value_status;
typedef struct {
    db_value_status status;
    db_source source;
    char label[32], value[DB_TEXT_CAP], detail[DB_TEXT_CAP], badge[16];
} db_widget_view;
/* Always overwrites output. Non-OK values are "--" with no leftover details.
 * Text is bounded ASCII for the host prototype; non-ASCII/control bytes -> '?'.
 * Unterminated input text is invalid. NULL output is a no-op. */
void db_widget_format(db_widget widget, const db_snapshot *snapshot,
                      uint64_t now_ms, db_widget_view *out);

/* Bounded host draw list. A zeroed frame has no commands and no valid canvas. */
typedef enum { DB_DRAW_RECT, DB_DRAW_TEXT } db_draw_kind;
typedef struct {
    db_draw_kind kind;
    cr_rect_t box; /* shared upstream source pixels, top-left origin */
    uint32_t rgb;
    int font_size;
    char text[DB_TEXT_CAP];
} db_draw_command;
typedef struct {
    int width, height;
    cr_rect_t clip;
    size_t count;
    db_draw_command commands[DB_COMMAND_CAP];
} db_frame;

typedef enum { DB_COMPACT, DB_EXTENDED } db_layout;
/* Explicit host preview preparation, NOT an activation/configuration API.
 * Integer source-pixel viewport, minimum 640x240, inside canvas <=4096x2160.
 * Returns 1 ready, 0 invalid; always clears previous output on failure.
 * Missing/stale fields render placeholders through the existing formatter.
 * NULL snapshot, unknown layout or unsupported geometry fails closed.
 * NULL output is a no-op.
 * out must not alias snapshot. No threads, allocations, I/O or vehicle routing. */
int db_layout_build(db_layout layout, const db_snapshot *snapshot, uint64_t now_ms,
                    int width, int height, cr_rect_t area, db_frame *out);
/* Checks canvas/clip, count, kinds, RGB, integer coordinates, bounded ASCII text
 * and primitive containment. This is host frame validation, not VC safety proof. */
int db_frame_valid(const db_frame *frame);
/* Validates whole frame before first byte; escapes XML and clips every text box.
 * Returns 1 success, 0 invalid/error. Caller owns FILE and checks flush/close errors.
 * Formatting uses integer coordinates, independent of floating-point locale. */
int db_frame_write_svg(FILE *stream, const db_frame *frame);

/* ---- P2-03: versioned host configuration (disabled by default) ----------- */
#define DB_CONFIG_VERSION 1
#define DB_CONFIG_TEXT_CAP 512 /* whole file, bytes; larger input is invalid */
#define DB_CONFIG_LINE_CAP 64  /* one line, excluding newline */

typedef struct {
    int version;      /* must equal DB_CONFIG_VERSION */
    int enabled;      /* 0 or 1; default 0 */
    db_layout layout; /* default DB_COMPACT; selecting it never enables */
} db_config;

/* Disabled Compact, current version. NULL is a no-op. */
void db_config_default(db_config *out);
/* 1 when every field is in range for the supported version, else 0. */
int db_config_valid(const db_config *config);
/* Strict "key=value" text, see docs/custom-dashboards/config.md. The first key
 * must be version=1; enabled and layout are optional (defaults apply). Unknown
 * or duplicate keys, whitespace, NUL bytes, oversize input/lines or bad values
 * are rejected. Returns 1 parsed; 0 invalid with out set to the default.
 * text may be NULL only with len 0 (empty input is invalid: no version). */
int db_config_parse(const char *text, size_t len, db_config *out);

typedef enum { DB_CONFIG_INVALID = -1, DB_CONFIG_MISSING = 0, DB_CONFIG_LOADED = 1 } db_config_load;
/* Host file loader: bounded read, then db_config_parse. Missing file and
 * invalid content both leave out as the disabled default. */
db_config_load db_config_load_path(const char *path, db_config *out);
/* Writes canonical text after validating. Returns 1 success, 0 invalid/error. */
int db_config_write(FILE *stream, const db_config *config);

/* Host configuration actions (candidate future GEM verbs; no GEM code exists).
 * Each returns 1 when applied to a valid config, 0 (and restores the default)
 * otherwise. Layout selection preserves enablement; disable preserves layout. */
int db_config_enable(db_config *config);
int db_config_disable(db_config *config);
int db_config_select_layout(db_config *config, db_layout layout);
void db_config_restore_default(db_config *config);

/* ---- P2-03: dashboard manager --------------------------------------------- */
/* Why the manager currently has no frame, or READY. Diagnostics only. */
typedef enum {
    DB_MANAGER_OFF_DISABLED, /* valid config with enabled=0 (the default) */
    DB_MANAGER_OFF_CONFIG,   /* invalid/missing-required config: default applied */
    DB_MANAGER_OFF_PROVIDER, /* no provider, or provider read failed */
    DB_MANAGER_OFF_GEOMETRY, /* layout rejected canvas/viewport */
    DB_MANAGER_READY
} db_manager_status;

/* Single-threaded, caller-owned state; no allocation, I/O or threads.
 * The provider is any db_provider (vehicle, mock, future composite); the manager
 * never selects a data source itself. Fields are private by convention. */
typedef struct {
    db_provider provider;
    db_config config;
    db_manager_status status;
    int config_error;     /* last set_config was rejected; cleared by actions */
    db_snapshot snapshot; /* one immutable snapshot per prepared frame */
    db_frame frame;       /* zeroed (count 0) whenever status != READY */
} db_manager;

/* Starts disabled with the default config and an empty frame. NULL provider is
 * allowed (updates then suppress output). NULL manager is a no-op. */
void db_manager_init(db_manager *manager, const db_provider *provider);
/* NULL or invalid config applies the default and records OFF_CONFIG, which
 * later updates keep reporting until a valid config or an explicit action.
 * Always clears the current frame; the next update decides output. */
void db_manager_set_config(db_manager *manager, const db_config *config);
/* Actions mirror db_config_* and clear the current frame immediately. */
void db_manager_disable(db_manager *manager);
void db_manager_restore_default(db_manager *manager);
void db_manager_select_layout(db_manager *manager, db_layout layout);
/* Reads the provider once and prepares a frame. Returns 1 READY; 0 suppressed
 * (frame cleared, snapshot cleared). Disabled config does not read the provider. */
int db_manager_update(db_manager *manager, uint64_t now_ms,
                      int width, int height, cr_rect_t area);
/* Current frame (count 0 when suppressed) or NULL for NULL manager. */
const db_frame *db_manager_frame(const db_manager *manager);
db_manager_status db_manager_get_status(const db_manager *manager);
const char *db_manager_status_name(db_manager_status status);

#ifdef __cplusplus
}
#endif
#endif
