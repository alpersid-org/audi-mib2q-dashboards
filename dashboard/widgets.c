/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <stdio.h>
#include <string.h>

static int terminated(const char *s)
{
    return memchr(s, 0, DB_TEXT_CAP) != NULL;
}

static void ascii_copy(char *out, const char *in)
{
    size_t i;
    for (i = 0; i < DB_TEXT_CAP - 1 && in[i]; ++i) {
        unsigned char c = (unsigned char)in[i];
        out[i] = c >= 32 && c <= 126 ? (char)c : '?';
    }
    out[i] = 0;
}

static const db_sample *sample_for(db_widget w, const db_snapshot *s)
{
    switch (w) {
    case DB_TEMPERATURE: return &s->temperature.sample;
    case DB_RANGE: return &s->range.sample;
    case DB_MEDIA: return &s->media.sample;
    case DB_NAVIGATION: case DB_NAV_DISTANCE: case DB_NAV_TIME: case DB_DESTINATION:
        return &s->navigation.sample;
    default: return NULL;
    }
}

/* A split navigation field the source did not supply (-1 / empty name) is
 * formatted as UNAVAILABLE, not INVALID: the route may legitimately lack it.
 * Layouts omit these views; direct formatter users retain the explicit status. */
static int not_supplied(db_widget w, const db_snapshot *s)
{
    switch (w) {
    case DB_NAVIGATION:
        return s->navigation.remaining_meters == -1 && s->navigation.remaining_seconds == -1;
    case DB_NAV_DISTANCE: return s->navigation.remaining_meters == -1;
    case DB_NAV_TIME: return s->navigation.remaining_seconds == -1;
    case DB_DESTINATION:
        return terminated(s->navigation.destination) && !s->navigation.destination[0];
    default: return 0;
    }
}

/* Host plausibility limits, NOT verified sensor/firmware limits. */
static int values_valid(db_widget w, const db_snapshot *s)
{
    switch (w) {
    case DB_TEMPERATURE:
        return s->temperature.decicelsius >= -1000 && s->temperature.decicelsius <= 1000;
    case DB_RANGE: return s->range.meters >= 0 && s->range.meters <= 10000000;
    case DB_MEDIA:
        return terminated(s->media.title) && s->media.title[0] && terminated(s->media.artist);
    case DB_NAVIGATION: /* each part may be -1 (not supplied); not both */
        return (s->navigation.remaining_meters == -1 ||
                (s->navigation.remaining_meters >= 0 &&
                 s->navigation.remaining_meters <= 1000000000)) &&
               (s->navigation.remaining_seconds == -1 ||
                (s->navigation.remaining_seconds >= 0 &&
                 s->navigation.remaining_seconds <= 31536000)) &&
               terminated(s->navigation.destination);
    case DB_NAV_DISTANCE:
        return s->navigation.remaining_meters >= 0 &&
               s->navigation.remaining_meters <= 1000000000;
    case DB_NAV_TIME:
        return s->navigation.remaining_seconds >= 0 &&
               s->navigation.remaining_seconds <= 31536000;
    case DB_DESTINATION:
        return terminated(s->navigation.destination) && s->navigation.destination[0];
    default: return 0;
    }
}

void db_widget_format(db_widget w, const db_snapshot *s, uint64_t now_ms, db_widget_view *out)
{
    static const char *const labels[] = {
        "OUTSIDE TEMPERATURE", "DRIVING RANGE", "NOW PLAYING", "NAVIGATION",
        "DISTANCE TO DESTINATION", "TIME TO DESTINATION", "DESTINATION"
    };
    const db_sample *sample;
    long v;
    if (!out) return;
    memset(out, 0, sizeof(*out));
    strcpy(out->value, "--");
    strcpy(out->badge, "UNAVAILABLE");
    if (w < DB_TEMPERATURE || w >= DB_WIDGET_COUNT) {
        out->status = DB_VALUE_INVALID;
        strcpy(out->badge, "INVALID"); return;
    }
    strcpy(out->label, labels[w]);
    if (!s) return;
    sample = sample_for(w, s);
    if (sample->source == DB_SOURCE_MOCK || sample->source == DB_SOURCE_VERIFIED ||
        sample->source == DB_SOURCE_VEHICLE)
        out->source = sample->source;
    if (sample->availability == DB_MISSING) return;
    if (sample->availability == DB_AVAILABLE && not_supplied(w, s)) return;
    if (sample->availability != DB_AVAILABLE || out->source == DB_SOURCE_UNKNOWN ||
        !values_valid(w, s)) {
        out->status = DB_VALUE_INVALID; strcpy(out->badge, "INVALID"); return;
    }
    if (sample->sampled_ms > now_ms) {
        out->status = DB_VALUE_FUTURE; strcpy(out->badge, "INVALID TIME"); return;
    }
    if (now_ms - sample->sampled_ms > DB_MAX_AGE_MS) {
        out->status = DB_VALUE_STALE; strcpy(out->badge, "STALE"); return;
    }
    out->status = DB_VALUE_OK;
    strcpy(out->badge, out->source == DB_SOURCE_MOCK ? "MOCK" :
                       out->source == DB_SOURCE_VEHICLE ? "UNVERIFIED" : "VERIFIED");
    switch (w) {
    case DB_TEMPERATURE:
        v = s->temperature.decicelsius;
        snprintf(out->value, sizeof(out->value), "%s%ld.%ld C", v < 0 ? "-" : "",
                 (v < 0 ? -v : v) / 10, (v < 0 ? -v : v) % 10);
        break;
    case DB_RANGE:
        snprintf(out->value, sizeof(out->value), "%ld km", (long)s->range.meters / 1000);
        strcpy(out->detail, "Estimated range"); break;
    case DB_MEDIA:
        ascii_copy(out->value, s->media.title); ascii_copy(out->detail, s->media.artist); break;
    case DB_NAVIGATION: {
        long m = s->navigation.remaining_meters, sec = s->navigation.remaining_seconds;
        if (m >= 0 && sec >= 0)
            snprintf(out->value, sizeof(out->value), "%ld.%ld km / %ld min",
                     m / 1000, m / 100 % 10, (sec + 59) / 60);
        else if (m >= 0)
            snprintf(out->value, sizeof(out->value), "%ld.%ld km", m / 1000, m / 100 % 10);
        else
            snprintf(out->value, sizeof(out->value), "%ld min", (sec + 59) / 60);
        ascii_copy(out->detail, s->navigation.destination); break;
    }
    case DB_NAV_DISTANCE:
        v = s->navigation.remaining_meters;
        snprintf(out->value, sizeof(out->value), "%ld.%ld km", v / 1000, v / 100 % 10);
        strcpy(out->detail, "Remaining route"); break;
    case DB_NAV_TIME:
        v = ((long)s->navigation.remaining_seconds + 59) / 60; /* round up, never optimistic */
        if (v < 60) snprintf(out->value, sizeof(out->value), "%ld min", v);
        else snprintf(out->value, sizeof(out->value), "%ld h %02ld min", v / 60, v % 60);
        strcpy(out->detail, "Estimated by navigation"); break;
    case DB_DESTINATION:
        ascii_copy(out->value, s->navigation.destination); break;
    default: break;
    }
}
