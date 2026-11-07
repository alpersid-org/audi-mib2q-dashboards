/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "../dashboard/dashboard.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static db_sample *sample_for(db_snapshot *s, db_widget w)
{
    switch (w) {
    case DB_TEMPERATURE: return &s->temperature.sample;
    case DB_RANGE: return &s->range.sample;
    case DB_MEDIA: return &s->media.sample;
    case DB_NAVIGATION: case DB_NAV_DISTANCE: case DB_NAV_TIME: case DB_DESTINATION:
        return &s->navigation.sample;
    default: assert(0); return NULL;
    }
}

static void expect(db_widget w, const db_snapshot *s, uint64_t now,
                   db_value_status status, const char *value, const char *badge)
{
    db_widget_view view;
    memset(&view, 'X', sizeof(view)); /* catch stale output surviving failure */
    db_widget_format(w, s, now, &view);
    assert(view.status == status);
    assert(!strcmp(view.value, value));
    assert(!strcmp(view.badge, badge));
    if (status != DB_VALUE_OK) assert(!view.detail[0]);
    assert(memchr(view.value, 0, sizeof(view.value)));
    assert(memchr(view.detail, 0, sizeof(view.detail)));
}

static void formatting(void)
{
    db_snapshot s;
    db_widget_view view;
    assert(db_mock_read(NULL, 100, &s));
    expect(DB_TEMPERATURE, &s, 100, DB_VALUE_OK, "12.5 C", "MOCK");
    expect(DB_RANGE, &s, 100, DB_VALUE_OK, "620 km", "MOCK");
    expect(DB_MEDIA, &s, 100, DB_VALUE_OK, "Night Drive", "MOCK");
    expect(DB_NAVIGATION, &s, 100, DB_VALUE_OK, "42.5 km / 38 min", "MOCK");
    expect(DB_NAV_DISTANCE, &s, 100, DB_VALUE_OK, "42.5 km", "MOCK");
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "38 min", "MOCK");
    expect(DB_DESTINATION, &s, 100, DB_VALUE_OK, "City Centre", "MOCK");
    db_widget_format(DB_NAVIGATION, &s, 100, &view);
    assert(!strcmp(view.detail, "City Centre"));
    db_widget_format(DB_NAV_DISTANCE, &s, 100, &view);
    assert(!strcmp(view.detail, "Remaining route") && !strcmp(view.label, "DISTANCE TO DESTINATION"));
    db_widget_format(DB_MEDIA, &s, 100, &view);
    assert(!strcmp(view.detail, "Studio Session"));
    assert(view.source == DB_SOURCE_MOCK);
    s.temperature.decicelsius = -1;
    expect(DB_TEMPERATURE, &s, 100, DB_VALUE_OK, "-0.1 C", "MOCK");
    s.temperature.decicelsius = -1000;
    expect(DB_TEMPERATURE, &s, 100, DB_VALUE_OK, "-100.0 C", "MOCK");
    s.temperature.decicelsius = 1000;
    expect(DB_TEMPERATURE, &s, 100, DB_VALUE_OK, "100.0 C", "MOCK");
    s.temperature.decicelsius = 0;
    s.range.meters = 0;
    s.navigation.remaining_meters = s.navigation.remaining_seconds = 0;
    expect(DB_TEMPERATURE, &s, 100, DB_VALUE_OK, "0.0 C", "MOCK");
    expect(DB_RANGE, &s, 100, DB_VALUE_OK, "0 km", "MOCK");
    expect(DB_NAVIGATION, &s, 100, DB_VALUE_OK, "0.0 km / 0 min", "MOCK");
    expect(DB_NAV_DISTANCE, &s, 100, DB_VALUE_OK, "0.0 km", "MOCK");
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "0 min", "MOCK");
    s.range.meters = 999; s.navigation.remaining_meters = 1999;
    expect(DB_RANGE, &s, 100, DB_VALUE_OK, "0 km", "MOCK"); /* floor, not optimism */
    expect(DB_NAV_DISTANCE, &s, 100, DB_VALUE_OK, "1.9 km", "MOCK");
    s.navigation.remaining_meters = 0;
    s.navigation.remaining_seconds = 1;
    expect(DB_NAVIGATION, &s, 100, DB_VALUE_OK, "0.0 km / 1 min", "MOCK");
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "1 min", "MOCK"); /* rounded up */
    s.navigation.remaining_seconds = 61;
    expect(DB_NAVIGATION, &s, 100, DB_VALUE_OK, "0.0 km / 2 min", "MOCK");
    s.navigation.remaining_seconds = 3540;                       /* 59 min */
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "59 min", "MOCK");
    s.navigation.remaining_seconds = 3541;                       /* rounds to 60 */
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "1 h 00 min", "MOCK");
    s.navigation.remaining_seconds = 3900;
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "1 h 05 min", "MOCK");
    s.navigation.remaining_seconds = 31536000;                  /* upper bound: 525600 min */
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "8760 h 00 min", "MOCK");
}

/* -1 / empty = not supplied by the source (aa_trip semantics, P2-07). */
static void not_supplied(void)
{
    db_snapshot s;
    assert(db_mock_read(NULL, 100, &s));
    s.navigation.remaining_seconds = -1;
    expect(DB_NAVIGATION, &s, 100, DB_VALUE_OK, "42.5 km", "MOCK");
    expect(DB_NAV_DISTANCE, &s, 100, DB_VALUE_OK, "42.5 km", "MOCK");
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_MISSING, "--", "UNAVAILABLE");
    s.navigation.remaining_seconds = 2280; s.navigation.remaining_meters = -1;
    expect(DB_NAVIGATION, &s, 100, DB_VALUE_OK, "38 min", "MOCK");
    expect(DB_NAV_DISTANCE, &s, 100, DB_VALUE_MISSING, "--", "UNAVAILABLE");
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "38 min", "MOCK");
    s.navigation.remaining_seconds = -1;
    expect(DB_NAVIGATION, &s, 100, DB_VALUE_MISSING, "--", "UNAVAILABLE");
    expect(DB_DESTINATION, &s, 100, DB_VALUE_OK, "City Centre", "MOCK");
    s.navigation.destination[0] = 0;
    expect(DB_DESTINATION, &s, 100, DB_VALUE_MISSING, "--", "UNAVAILABLE");
    /* not-supplied applies only to an otherwise valid sample */
    s.navigation.sample.availability = DB_INVALID;
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_INVALID, "--", "INVALID");
    /* -2 is not "not supplied": invalid */
    assert(db_mock_read(NULL, 100, &s));
    s.navigation.remaining_meters = -2;
    expect(DB_NAVIGATION, &s, 100, DB_VALUE_INVALID, "--", "INVALID");
    expect(DB_NAV_DISTANCE, &s, 100, DB_VALUE_INVALID, "--", "INVALID");
    expect(DB_NAV_TIME, &s, 100, DB_VALUE_OK, "38 min", "MOCK"); /* own field only */
}

/* P2-08 provenance: live vehicle data is badged UNVERIFIED until on-car validation. */
static void vehicle_provenance(void)
{
    db_snapshot s;
    db_widget_view view;
    int i;
    assert(db_mock_read(NULL, 100, &s));
    for (i = 0; i < DB_WIDGET_COUNT; ++i) {
        sample_for(&s, (db_widget)i)->source = DB_SOURCE_VEHICLE;
        db_widget_format((db_widget)i, &s, 100, &view);
        assert(view.status == DB_VALUE_OK && view.source == DB_SOURCE_VEHICLE);
        assert(!strcmp(view.badge, "UNVERIFIED"));
    }
}

static void sample_states(void)
{
    db_snapshot s;
    int i;
    for (i = 0; i < DB_WIDGET_COUNT; ++i) {
        db_widget w = (db_widget)i;
        db_sample *m;
        db_widget_view view;
        memset(&s, 0, sizeof(s));
        expect(w, &s, 100, DB_VALUE_MISSING, "--", "UNAVAILABLE");
        expect(w, NULL, 100, DB_VALUE_MISSING, "--", "UNAVAILABLE");
        assert(db_mock_read(NULL, 100, &s));
        m = sample_for(&s, w);
        m->availability = DB_INVALID;
        expect(w, &s, 100, DB_VALUE_INVALID, "--", "INVALID");
        m->availability = (db_availability)99;
        expect(w, &s, 100, DB_VALUE_INVALID, "--", "INVALID");
        m->availability = DB_AVAILABLE; m->source = DB_SOURCE_UNKNOWN;
        expect(w, &s, 100, DB_VALUE_INVALID, "--", "INVALID");
        m->source = (db_source)99;
        expect(w, &s, 100, DB_VALUE_INVALID, "--", "INVALID");
        m->source = DB_SOURCE_VERIFIED; /* interface branch, NOT a real provider */
        db_widget_format(w, &s, 100, &view);
        assert(view.status == DB_VALUE_OK && !strcmp(view.badge, "VERIFIED"));
        m->source = DB_SOURCE_MOCK;
        db_widget_format(w, &s, 100 + DB_MAX_AGE_MS, &view);
        assert(view.status == DB_VALUE_OK); /* inclusive age boundary */
        expect(w, &s, 101 + DB_MAX_AGE_MS, DB_VALUE_STALE, "--", "STALE");
        expect(w, &s, 99, DB_VALUE_FUTURE, "--", "INVALID TIME");
        m->sampled_ms = UINT64_MAX;
        expect(w, &s, 0, DB_VALUE_FUTURE, "--", "INVALID TIME");
        m->sampled_ms = 0;
        db_widget_format(w, &s, 0, &view);
        assert(view.status == DB_VALUE_OK); /* zero is a valid clock epoch */
        expect(w, &s, UINT64_MAX, DB_VALUE_STALE, "--", "STALE");
    }
    expect((db_widget)-1, NULL, 0, DB_VALUE_INVALID, "--", "INVALID");
    expect(DB_WIDGET_COUNT, NULL, 0, DB_VALUE_INVALID, "--", "INVALID");
    db_widget_format(DB_RANGE, NULL, 0, NULL);
}

static void invalid_values(void)
{
    db_snapshot s;
    int32_t *numbers[6];
    const db_widget widgets[] = { DB_TEMPERATURE, DB_RANGE, DB_NAVIGATION,
                                  DB_NAVIGATION, DB_NAV_DISTANCE, DB_NAV_TIME };
    /* navigation parts: -1 means not supplied, so the first invalid value is -2 */
    const int32_t lower[] = { -1000, 0, -1, -1, -1, -1 };
    const int32_t upper[] = { 1000, 10000000, 1000000000, 31536000, 1000000000, 31536000 };
    size_t i, j;
    assert(db_mock_read(NULL, 0, &s));
    numbers[0] = &s.temperature.decicelsius; numbers[1] = &s.range.meters;
    numbers[2] = &s.navigation.remaining_meters; numbers[3] = &s.navigation.remaining_seconds;
    numbers[4] = &s.navigation.remaining_meters; numbers[5] = &s.navigation.remaining_seconds;
    for (i = 0; i < 6; ++i) {
        int32_t original = *numbers[i];
        int32_t bad[] = { lower[i] - 1, upper[i] + 1, INT32_MIN, INT32_MAX };
        db_widget_view view;
        for (j = 0; j < sizeof(bad)/sizeof(bad[0]); ++j) {
            *numbers[i] = bad[j];
            expect(widgets[i], &s, 0, DB_VALUE_INVALID, "--", "INVALID");
        }
        *numbers[i] = lower[i];
        db_widget_format(widgets[i], &s, 0, &view);
        /* -1 renders as the other part (NAVIGATION) or UNAVAILABLE (split cards) */
        assert(view.status == (lower[i] == -1 && widgets[i] != DB_NAVIGATION
                               ? DB_VALUE_MISSING : DB_VALUE_OK));
        *numbers[i] = upper[i];
        db_widget_format(widgets[i], &s, 0, &view); assert(view.status == DB_VALUE_OK);
        *numbers[i] = original;
    }
}

static void text_bounds(void)
{
    db_snapshot s;
    db_widget_view view;
    char *fields[3];
    size_t i;
    assert(db_mock_read(NULL, 0, &s));
    fields[0] = s.media.title; fields[1] = s.media.artist;
    fields[2] = s.navigation.destination;
    for (i = 0; i < 3; ++i) {
        char saved[DB_TEXT_CAP];
        db_widget w = i < 2 ? DB_MEDIA : DB_NAVIGATION;
        memcpy(saved, fields[i], sizeof(saved));
        memset(fields[i], 'A', DB_TEXT_CAP);
        expect(w, &s, 0, DB_VALUE_INVALID, "--", "INVALID");
        fields[i][DB_TEXT_CAP - 1] = 0;
        db_widget_format(w, &s, 0, &view);
        assert(view.status == DB_VALUE_OK);
        assert(strlen(i == 0 ? view.value : view.detail) == DB_TEXT_CAP - 1);
        memcpy(fields[i], saved, sizeof(saved));
    }
    s.media.title[0] = 0;
    expect(DB_MEDIA, &s, 0, DB_VALUE_INVALID, "--", "INVALID");
    strcpy(s.media.title, "A\n\001\177\377Z");
    expect(DB_MEDIA, &s, 0, DB_VALUE_OK, "A????Z", "MOCK");
    s.media.artist[0] = 0; s.navigation.destination[0] = 0; /* optional strings */
    db_widget_format(DB_MEDIA, &s, 0, &view);
    assert(view.status == DB_VALUE_OK && !view.detail[0]);
    db_widget_format(DB_NAVIGATION, &s, 0, &view);
    assert(view.status == DB_VALUE_OK && !view.detail[0]);
    expect(DB_DESTINATION, &s, 0, DB_VALUE_MISSING, "--", "UNAVAILABLE");
    memset(s.navigation.destination, 'D', DB_TEXT_CAP);              /* unterminated */
    expect(DB_DESTINATION, &s, 0, DB_VALUE_INVALID, "--", "INVALID");
}

static int partial_read(void *context, uint64_t now, db_snapshot *out)
{
    int result = *(int *)context;
    out->range.sample.availability = DB_AVAILABLE;
    out->range.sample.source = DB_SOURCE_MOCK;
    out->range.sample.sampled_ms = now;
    out->range.meters = 1000;
    return result;
}

static void providers(void)
{
    db_provider mock = { db_mock_read, NULL }, partial;
    db_snapshot s, old;
    int result = 1, i;
    partial.read = partial_read; partial.context = &result;
    assert(db_snapshot_read(&mock, 42, &old));
    for (i = 0; i < 100; ++i) {
        int w;
        assert(db_snapshot_read(&mock, (uint64_t)i, &s));
        for (w = 0; w < DB_WIDGET_COUNT; ++w) {
            db_widget_view a, b;
            assert(sample_for(&s, (db_widget)w)->sampled_ms == (uint64_t)i);
            db_widget_format((db_widget)w, &s, (uint64_t)i, &a);
            db_widget_format((db_widget)w, &old, 42, &b);
            assert(a.status == DB_VALUE_OK && a.source == DB_SOURCE_MOCK);
            assert(!strcmp(a.value, b.value) && !strcmp(a.detail, b.detail));
        }
    }
    assert(db_snapshot_read(&partial, 50, &s));
    expect(DB_RANGE, &s, 50, DB_VALUE_OK, "1 km", "MOCK");
    expect(DB_MEDIA, &s, 50, DB_VALUE_MISSING, "--", "UNAVAILABLE");
    for (result = -1; result <= 2; ++result) {
        if (result == 1) continue;
        assert(db_snapshot_read(&mock, 50, &s));
        assert(!db_snapshot_read(&partial, 50, &s));
        for (i = 0; i < DB_WIDGET_COUNT; ++i)
            expect((db_widget)i, &s, 50, DB_VALUE_MISSING, "--", "UNAVAILABLE");
        assert(s.range.meters == 0 && !s.media.title[0]);
    }
    assert(!db_snapshot_read(NULL, 0, &s));
    partial.read = NULL; assert(!db_snapshot_read(&partial, 0, &s));
    assert(!db_snapshot_read(&mock, 0, NULL));
    assert(!db_mock_read(NULL, 0, NULL));
}

int main(void)
{
    db_frame empty = {0};
    assert(empty.count == 0);
    formatting(); not_supplied(); vehicle_provenance(); sample_states(); invalid_values();
    text_bounds(); providers();
    puts("dashboard_test: units/zero, not-supplied nav parts, H/MM time, VEHICLE provenance, validity, freshness, bounds, text, provider clearing and mock determinism PASS");
    return 0;
}
