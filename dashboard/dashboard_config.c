/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <string.h>

/* Host config tool. Reads CONFIG if given (missing -> default), applies one
 * action and prints canonical config to stdout; never writes files itself.
 * Layout actions never change enablement. No GEM, vehicle or provider access. */
int main(int argc, char **argv)
{
    db_config c;
    db_config_load load = DB_CONFIG_MISSING;
    const char *a = argc >= 2 ? argv[1] : "";
    int ok = 1;
    if (argc < 2 || argc > 3 ||
        (strcmp(a, "show") && strcmp(a, "enable") && strcmp(a, "disable") &&
         strcmp(a, "compact") && strcmp(a, "extended") && strcmp(a, "restore-default"))) {
        fputs("Usage: dashboard_config show|enable|disable|compact|extended|restore-default"
              " [CONFIG] > new.cfg\nHost configuration only; no vehicle output.\n", stderr);
        return 2;
    }
    if (argc == 3) load = db_config_load_path(argv[2], &c);
    else db_config_default(&c);
    if (load == DB_CONFIG_MISSING && argc == 3)
        fputs("dashboard_config: config missing; using disabled default\n", stderr);
    if (load == DB_CONFIG_INVALID)
        fputs("dashboard_config: config invalid; using disabled default\n", stderr);
    if (!strcmp(a, "enable")) ok = db_config_enable(&c);
    else if (!strcmp(a, "disable")) ok = db_config_disable(&c);
    else if (!strcmp(a, "compact")) ok = db_config_select_layout(&c, DB_COMPACT);
    else if (!strcmp(a, "extended")) ok = db_config_select_layout(&c, DB_EXTENDED);
    else if (!strcmp(a, "restore-default")) db_config_restore_default(&c);
    if (!ok || !db_config_write(stdout, &c) || fflush(stdout)) return 1;
    /* show reports invalid input as failure; other actions recover from default. */
    return (!strcmp(a, "show") && load == DB_CONFIG_INVALID) ? 1 : 0;
}
