/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <stdio.h>

int main(void)
{
    db_provider provider = { db_mock_read, NULL };
    db_snapshot snapshot;
    db_widget_view view;
    int i;
    const uint64_t now_ms = 1000; /* deterministic host clock, not wall/vehicle time */
    puts("HOST PROTOTYPE - MOCK DATA - NO VEHICLE OUTPUT");
    if (!db_snapshot_read(&provider, now_ms, &snapshot)) return 1;
    for (i = 0; i < DB_WIDGET_COUNT; ++i) {
        db_widget_format((db_widget)i, &snapshot, now_ms, &view);
        printf("%-20s [%s] %s%s%s\n", view.label, view.badge, view.value,
               view.detail[0] ? " | " : "", view.detail);
    }
    return ferror(stdout) ? 1 : 0;
}
