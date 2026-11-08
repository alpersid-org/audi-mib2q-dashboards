/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <string.h>

/* Host config tool. Reads CONFIG if given (missing -> default), applies one
 * action and prints canonical config to stdout. With -w (P2-C07) it also stores
 * the result atomically back to CONFIG (db_config_store_path); "show" cannot be
 * combined with -w. Layout actions never change enablement. No GEM, vehicle or
 * provider access. */
int main(int argc, char **argv)
{
    db_config c;
    db_config_load load = DB_CONFIG_MISSING;
    int write_back = argc >= 2 && !strcmp(argv[1], "-w");
    const char *a;
    int ok = 1;
    if (write_back) { argv++; argc--; }
    a = argc >= 2 ? argv[1] : "";
    if (argc < 2 || argc > 3 || (write_back && (argc != 3 || !strcmp(a, "show"))) ||
        (strcmp(a, "show") && strcmp(a, "enable") && strcmp(a, "disable") &&
         strcmp(a, "compact") && strcmp(a, "extended") && strcmp(a, "restore-default"))) {
        fputs("Usage: dashboard_config show|enable|disable|compact|extended|restore-default"
              " [CONFIG] > new.cfg\n"
              "       dashboard_config -w enable|disable|compact|extended|restore-default CONFIG\n"
              "Host configuration only; no vehicle output.\n", stderr);
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
    if (write_back && !db_config_store_path(argv[2], &c)) {
        fputs("dashboard_config: atomic store failed; file unchanged\n", stderr);
        return 1;
    }
    /* show reports invalid input as failure; other actions recover from default. */
    return (!strcmp(a, "show") && load == DB_CONFIG_INVALID) ? 1 : 0;
}
