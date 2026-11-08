/* SPDX-License-Identifier: GPL-3.0-or-later */
package com.luka.carplay.dashboard;

import java.io.FileInputStream;
import java.io.File;

/**
 * P2-08 custom dashboard enable gate.  Reads the same strict version-1 text format as
 * dashboard/config.c (docs/custom-dashboards/config.md): first key "version=1", optional
 * "enabled=0|1" and "layout=compact|extended", each at most once, '#' comment and blank
 * lines, no whitespace, LF (CR tolerated), whole file <= 512 bytes, lines <= 64 bytes.
 * Anything else, a missing file or a read error means DISABLED.  Pure Java 1.4.
 */
public final class DashboardConfig {
    public static final String PATH = "/mnt/app/carplay_dashboard.cfg";
    static final int TEXT_CAP = 512, LINE_CAP = 64;
    /* Host tests point this at a temporary file; production always uses PATH. */
    static volatile String path = PATH;

    private DashboardConfig() { }

    /** True only for a valid file at PATH with enabled=1.  Never throws. */
    public static boolean enabled() {
        return enabled(read(path));
    }

    static byte[] read(String path) {
        FileInputStream in = null;
        try {
            /* Missing, directory or FIFO must not block the lifecycle worker.
             * Java 1.4 cannot provide an atomic no-follow open; target filesystem
             * trust/permissions remain a deployment prerequisite. */
            File file = new File(path).getAbsoluteFile();
            if (!file.isFile() || !file.getCanonicalFile().equals(
                    new File(file.getParentFile().getCanonicalFile(), file.getName()))) return null;
            in = new FileInputStream(path);
            byte[] buf = new byte[TEXT_CAP + 1];         /* one extra byte detects oversize */
            int n = 0, r;
            while (n < buf.length && (r = in.read(buf, n, buf.length - n)) > 0) n += r;
            if (n > TEXT_CAP) return null;
            byte[] out = new byte[n];
            System.arraycopy(buf, 0, out, 0, n);
            return out;
        } catch (Throwable t) {
            return null;
        } finally {
            if (in != null) try { in.close(); } catch (Throwable t) { }
        }
    }

    /** Strict parse; returns true only when the text is valid and enabled=1. */
    static boolean enabled(byte[] text) {
        if (text == null || text.length == 0 || text.length > TEXT_CAP) return false;
        boolean version = false, seenEnabled = false, seenLayout = false, on = false;
        int pos = 0;
        while (pos < text.length) {
            int end = pos;
            while (end < text.length && text[end] != '\n') end++;
            int next = end < text.length ? end + 1 : end;
            int n = end - pos;
            if (n > 0 && text[pos + n - 1] == '\r') n--;
            if (n > LINE_CAP) return false;
            if (n > 0 && text[pos] != '#') {
                int eq = -1;
                for (int i = pos; i < pos + n; i++) {
                    int c = text[i] & 0xff;
                    if (c <= 0x20 || c >= 0x7f) return false;
                    if (c == '=' && eq < 0) eq = i;
                }
                if (eq < 0) return false;
                String key = ascii(text, pos, eq - pos);
                String value = ascii(text, eq + 1, pos + n - eq - 1);
                if (!version) {
                    if (!key.equals("version") || !value.equals("1")) return false;
                    version = true;
                } else if (key.equals("enabled") && !seenEnabled) {
                    seenEnabled = true;
                    if (value.equals("1")) on = true;
                    else if (!value.equals("0")) return false;
                } else if (key.equals("layout") && !seenLayout) {
                    seenLayout = true;
                    if (!value.equals("compact") && !value.equals("extended")) return false;
                } else {
                    return false;                          /* unknown or duplicate key */
                }
            }
            /* NUL anywhere (comments included) rejects the file, as in config.c */
            for (int i = pos; i < end; i++) if (text[i] == 0) return false;
            pos = next;
        }
        return version && on;
    }

    private static String ascii(byte[] b, int off, int len) {
        char[] c = new char[len];
        for (int i = 0; i < len; i++) c[i] = (char)(b[off + i] & 0x7f);
        return new String(c);
    }
}
