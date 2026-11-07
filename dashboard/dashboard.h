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
/* VERIFIED is reserved for a future independently validated source; none ships. */
typedef enum { DB_SOURCE_UNKNOWN, DB_SOURCE_MOCK, DB_SOURCE_VERIFIED } db_source;
typedef struct {
    db_availability availability;
    db_source source;
    uint64_t sampled_ms; /* same monotonic clock/epoch as caller's now_ms; 0 valid */
} db_sample;

typedef struct { db_sample sample; int32_t decicelsius; } db_temperature;
typedef struct { db_sample sample; int32_t meters; } db_distance;
typedef struct { db_sample sample; int32_t deciliters_per_100km; } db_consumption;
typedef struct {
    db_sample sample;
    char title[DB_TEXT_CAP], artist[DB_TEXT_CAP];
} db_media;
/* Atomic group: all three fields must describe the same destination/estimate.
 * Future adapters must use the OLDEST contributing sample time, never refresh
 * old estimates just because a new destination string arrived. */
typedef struct {
    db_sample sample;
    int32_t remaining_meters, remaining_seconds;
    char destination[DB_TEXT_CAP];
} db_navigation;
typedef struct {
    db_temperature temperature;
    db_distance range, trip;
    db_consumption consumption;
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

typedef enum {
    DB_TEMPERATURE, DB_RANGE, DB_TRIP, DB_CONSUMPTION, DB_MEDIA, DB_NAVIGATION,
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

#ifdef __cplusplus
}
#endif
#endif
