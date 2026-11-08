/* SPDX-License-Identifier: GPL-3.0-or-later */
package com.luka.carplay.dashboard;

/**
 * P2-C05 typed settings shared with the C side (docs/custom-dashboards/settings-contract.md).
 * Mirrors db_config: version 1, enabled, layout. Layout values equal C's db_layout
 * (DB_COMPACT = 0, DB_EXTENDED = 1).  Immutable.  valid == false is the disabled
 * Compact default produced for a missing, unreadable or invalid file; a layout in
 * an invalid file is never honoured.  Pure Java 1.4.
 */
public final class DashboardSettings {
    public static final int VERSION = 1;
    public static final int LAYOUT_COMPACT = 0, LAYOUT_EXTENDED = 1;
    /** Disabled Compact default (C: db_config_default). */
    public static final DashboardSettings DEFAULT = new DashboardSettings(false, false, LAYOUT_COMPACT);

    public final boolean valid, enabled;
    public final int layout;

    DashboardSettings(boolean valid, boolean enabled, int layout) {
        this.valid = valid;
        this.enabled = valid && enabled;
        this.layout = valid && layout == LAYOUT_EXTENDED ? LAYOUT_EXTENDED : LAYOUT_COMPACT;
    }

    public String toString() {
        return "DashboardSettings[valid=" + valid + " enabled=" + enabled + " layout="
            + (layout == LAYOUT_EXTENDED ? "extended" : "compact") + "]";
    }
}
