/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <string.h>

/* Explicit mock-only export to stdout; shell redirection chooses output path.
 * No default render, no config files, no activation markers or live provider. */
int main(int argc, char **argv)
{
    db_provider provider = { db_mock_read, NULL };
    db_snapshot s;
    db_frame frame;
    db_layout layout;
    int w, h;
    if (argc != 3 || (strcmp(argv[1], "compact") && strcmp(argv[1], "extended")) ||
        (strcmp(argv[2], "wide") && strcmp(argv[2], "tall"))) {
        fputs("Usage: dashboard_preview compact|extended wide|tall > preview.svg\nMock host preview only; no vehicle output.\n", stderr);
        return 2;
    }
    layout = !strcmp(argv[1], "compact") ? DB_COMPACT : DB_EXTENDED;
    w = !strcmp(argv[2], "wide") ? 800 : 640;
    h = !strcmp(argv[2], "wide") ? 252 : 360;
    if (!db_snapshot_read(&provider, 1000, &s) ||
        !db_layout_build(layout, &s, 1000, w, h, (cr_rect_t){0, 0, (float)w, (float)h}, &frame) ||
        !db_frame_write_svg(stdout, &frame) || fflush(stdout)) return 1;
    return 0;
}
