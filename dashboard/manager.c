/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-03 dashboard manager: config gate + one provider read + layout build.
 * Source-agnostic: the caller injects any db_provider. Every non-ready path
 * clears both the snapshot and the frame so no stale drawing survives. */
#include "dashboard.h"
#include <string.h>

static void suppress(db_manager *m, db_manager_status why)
{
    memset(&m->snapshot, 0, sizeof(m->snapshot));
    memset(&m->frame, 0, sizeof(m->frame));
    m->status = why;
}

void db_manager_init(db_manager *m, const db_provider *provider)
{
    if (!m) return;
    memset(m, 0, sizeof(*m));
    if (provider) m->provider = *provider;
    db_config_default(&m->config);
    suppress(m, DB_MANAGER_OFF_DISABLED);
}

void db_manager_set_config(db_manager *m, const db_config *config)
{
    if (!m) return;
    m->config_error = !db_config_valid(config);
    if (!m->config_error) {
        m->config = *config;
        suppress(m, DB_MANAGER_OFF_DISABLED); /* next update decides output */
    } else {
        db_config_default(&m->config);
        suppress(m, DB_MANAGER_OFF_CONFIG);
    }
}

void db_manager_disable(db_manager *m)
{
    if (!m) return;
    m->config_error = 0;
    if (!db_config_disable(&m->config)) { suppress(m, DB_MANAGER_OFF_CONFIG); return; }
    suppress(m, DB_MANAGER_OFF_DISABLED);
}

void db_manager_restore_default(db_manager *m)
{
    if (!m) return;
    m->config_error = 0;
    db_config_restore_default(&m->config);
    suppress(m, DB_MANAGER_OFF_DISABLED);
}

void db_manager_select_layout(db_manager *m, db_layout layout)
{
    if (!m) return;
    m->config_error = 0;
    if (!db_config_select_layout(&m->config, layout)) {
        suppress(m, DB_MANAGER_OFF_CONFIG);
        return;
    }
    /* Clear now: an old Compact frame must not be shown as Extended (or vice
     * versa). Enablement is unchanged; the next update rebuilds if enabled. */
    suppress(m, DB_MANAGER_OFF_DISABLED);
}

int db_manager_update(db_manager *m, uint64_t now_ms, int width, int height,
                      cr_rect_t area)
{
    if (!m) return 0;
    if (m->config_error || !db_config_valid(&m->config)) {
        if (!db_config_valid(&m->config)) db_config_default(&m->config);
        suppress(m, DB_MANAGER_OFF_CONFIG); /* default is disabled: no read */
        return 0;
    }
    if (!m->config.enabled) { suppress(m, DB_MANAGER_OFF_DISABLED); return 0; }
    if (!db_snapshot_read(&m->provider, now_ms, &m->snapshot)) {
        suppress(m, DB_MANAGER_OFF_PROVIDER);
        return 0;
    }
    if (!db_layout_widget_count(m->config.layout, &m->snapshot, now_ms)) {
        suppress(m, DB_MANAGER_OFF_DATA);
        return 0;
    }
    if (!db_layout_build(m->config.layout, &m->snapshot, now_ms, width, height,
                         area, &m->frame)) {
        suppress(m, DB_MANAGER_OFF_GEOMETRY);
        return 0;
    }
    m->status = DB_MANAGER_READY;
    return 1;
}

const db_frame *db_manager_frame(const db_manager *m)
{
    return m ? &m->frame : NULL;
}

db_manager_status db_manager_get_status(const db_manager *m)
{
    return m ? m->status : DB_MANAGER_OFF_CONFIG;
}

const char *db_manager_status_name(db_manager_status s)
{
    switch (s) {
    case DB_MANAGER_OFF_DISABLED: return "disabled";
    case DB_MANAGER_OFF_CONFIG: return "invalid-config";
    case DB_MANAGER_OFF_PROVIDER: return "provider-failure";
    case DB_MANAGER_OFF_DATA: return "no-data";
    case DB_MANAGER_OFF_GEOMETRY: return "unsupported-geometry";
    case DB_MANAGER_READY: return "ready";
    }
    return "unknown";
}
