/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <string.h>

int db_snapshot_read(const db_provider *provider, uint64_t now_ms, db_snapshot *out)
{
    db_snapshot candidate;
    if (!out) return 0;
    memset(&candidate, 0, sizeof(candidate));
    if (!provider || !provider->read ||
        provider->read(provider->context, now_ms, &candidate) != 1) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    *out = candidate;
    return 1;
}
