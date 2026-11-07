/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <string.h>

/* Runs the manager with an explicitly chosen MOCK provider and a config file.
 * Writes SVG only when the manager is READY; otherwise writes nothing to stdout,
 * reports the suppression reason on stderr and exits 3. The mock is selected
 * here, in the host CLI; the manager itself has no built-in data source. */
int main(int argc, char **argv)
{
    db_provider mock = { db_mock_read, NULL };
    db_manager m;
    db_config c;
    db_config_load load;
    int w, h;
    if (argc != 3 || (strcmp(argv[2], "wide") && strcmp(argv[2], "tall"))) {
        fputs("Usage: dashboard_manager_preview CONFIG wide|tall > frame.svg\n"
              "Mock host preview through the manager; no vehicle output.\n", stderr);
        return 2;
    }
    w = !strcmp(argv[2], "wide") ? 800 : 640;
    h = !strcmp(argv[2], "wide") ? 252 : 360;
    db_manager_init(&m, &mock);
    load = db_config_load_path(argv[1], &c);
    db_manager_set_config(&m, load == DB_CONFIG_LOADED ? &c : NULL);
    if (load == DB_CONFIG_MISSING)
        fputs("dashboard_manager_preview: config missing; disabled default\n", stderr);
    if (!db_manager_update(&m, 1000, w, h, (cr_rect_t){0, 0, (float)w, (float)h})) {
        fprintf(stderr, "dashboard_manager_preview: suppressed (%s); no output\n",
                db_manager_status_name(db_manager_get_status(&m)));
        return 3;
    }
    if (!db_frame_write_svg(stdout, db_manager_frame(&m)) || fflush(stdout)) return 1;
    return 0;
}
