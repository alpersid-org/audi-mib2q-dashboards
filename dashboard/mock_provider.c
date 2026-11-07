/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <string.h>

int db_mock_read(void *context, uint64_t now_ms, db_snapshot *out)
{
    db_sample sample;
    (void)context;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    memset(&sample, 0, sizeof(sample));
    sample.availability = DB_AVAILABLE;
    sample.source = DB_SOURCE_MOCK;
    sample.sampled_ms = now_ms;
    /* Mock is for automated tests and development only (scope req. 6). */
    out->temperature.sample = out->range.sample = sample;
    out->media.sample = out->navigation.sample = sample;
    out->temperature.decicelsius = 125;
    out->range.meters = 620000;
    strcpy(out->media.title, "Night Drive");
    strcpy(out->media.artist, "Studio Session");
    out->navigation.remaining_meters = 42500;
    out->navigation.remaining_seconds = 2280;
    strcpy(out->navigation.destination, "City Centre");
    return 1;
}
