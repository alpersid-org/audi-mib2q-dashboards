/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Bounded host preparation benchmark (P2-04). Measures this host only; it says
 * nothing about head-unit CPU, GPU or frame rate. Fixed iteration count. */
#define _POSIX_C_SOURCE 199309L
#include "dashboard.h"
#include <stdio.h>
#include <time.h>

#define ITERATIONS 20000

static double now_us(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1e6 + (double)t.tv_nsec / 1e3;
}

static int bench(const char *name, db_layout layout, int w, int h)
{
    db_provider mock = { db_mock_read, NULL };
    db_manager m;
    db_config c;
    FILE *sink = fopen("/dev/null", "wb");
    double start, prep, svg;
    int i;
    if (!sink) return 0;
    db_manager_init(&m, &mock);
    db_config_default(&c); c.enabled = 1; c.layout = layout;
    db_manager_set_config(&m, &c);
    start = now_us();
    for (i = 0; i < ITERATIONS; ++i)
        if (!db_manager_update(&m, (uint64_t)i, w, h, (cr_rect_t){0, 0, (float)w, (float)h})) {
            fclose(sink); return 0;
        }
    prep = (now_us() - start) / ITERATIONS;
    start = now_us();
    for (i = 0; i < ITERATIONS / 10; ++i)
        if (!db_frame_write_svg(sink, db_manager_frame(&m))) { fclose(sink); return 0; }
    svg = (now_us() - start) / (ITERATIONS / 10);
    fclose(sink);
    printf("%-14s %4dx%-4d prepare %7.2f us/frame  svg %7.2f us/frame  (%zu commands)\n",
           name, w, h, prep, svg, db_manager_frame(&m)->count);
    return 1;
}

int main(void)
{
    puts("HOST BENCHMARK - MOCK DATA - not a head-unit performance claim");
    printf("iterations: %d prepare, %d svg; sizeof(db_manager)=%zu bytes\n",
           ITERATIONS, ITERATIONS / 10, sizeof(db_manager));
    return bench("compact-wide", DB_COMPACT, 800, 252) &&
        bench("compact-tall", DB_COMPACT, 640, 360) &&
        bench("extended-wide", DB_EXTENDED, 800, 252) &&
        bench("extended-tall", DB_EXTENDED, 640, 360) ? 0 : 1;
}
