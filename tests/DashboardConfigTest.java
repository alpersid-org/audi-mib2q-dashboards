package com.luka.carplay.dashboard;

import java.io.File;
import java.io.FileOutputStream;

/* P2-FINAL: the HMI enable gate must follow the same strict v1 rules as
 * dashboard/config.c (tests/dashboard_manager_test.c uses the same cases). */
public final class DashboardConfigTest {
    private static void check(boolean ok, String message) {
        if (!ok) throw new AssertionError(message);
    }
    private static byte[] b(String s) throws Exception { return s.getBytes("ISO-8859-1"); }

    /* P2-C05: shared conformance vectors (same file as dashboard_config_vectors_test.c). */
    static int vectors(String path) throws Exception {
        java.io.BufferedReader r = new java.io.BufferedReader(
            new java.io.InputStreamReader(new java.io.FileInputStream(path), "US-ASCII"));
        String line, name = null, expect = null;
        int cases = 0;
        try {
            while ((line = r.readLine()) != null) {
                if (line.length() == 0 || line.charAt(0) == '#') continue;
                if (line.startsWith("case ")) {
                    check(name == null, "vector without hex: " + line);
                    int sp = line.indexOf(' ', 5);
                    name = line.substring(5, sp);
                    expect = line.substring(sp + 1);
                    continue;
                }
                check(name != null && line.startsWith("hex "), "bad vector line: " + line);
                String hex = line.substring(4);
                byte[] data = new byte[hex.equals("-") ? 0 : hex.length() / 2];
                for (int i = 0; i < data.length; i++)
                    data[i] = (byte) Integer.parseInt(hex.substring(2 * i, 2 * i + 2), 16);
                DashboardSettings got = DashboardConfig.parse(data);
                if (expect.equals("invalid")) {
                    check(!got.valid && !got.enabled && got.layout == DashboardSettings.LAYOUT_COMPACT,
                        "vector " + name + ": accepted, expected invalid (" + got + ")");
                } else {
                    boolean en = expect.indexOf("enabled=1") >= 0;
                    int lay = expect.endsWith("layout=extended") ? DashboardSettings.LAYOUT_EXTENDED
                                                                 : DashboardSettings.LAYOUT_COMPACT;
                    check(expect.startsWith("valid ") && got.valid && got.enabled == en && got.layout == lay,
                        "vector " + name + ": got " + got + ", expected " + expect);
                }
                check(DashboardConfig.enabled(data) == got.enabled, "enabled() differs from parse()");
                name = null;
                cases++;
            }
        } finally { r.close(); }
        check(name == null && cases >= 40, "vector count " + cases);
        return cases;
    }

    public static void main(String[] args) throws Exception {
        int nv = vectors(args[1]);
        check(DashboardSettings.LAYOUT_COMPACT == 0 && DashboardSettings.LAYOUT_EXTENDED == 1,
            "layout values must equal C db_layout");
        check(!DashboardSettings.DEFAULT.valid && !DashboardSettings.DEFAULT.enabled
            && DashboardSettings.DEFAULT.layout == DashboardSettings.LAYOUT_COMPACT, "default");
        check(!new DashboardSettings(false, true, 1).enabled
            && new DashboardSettings(false, true, 1).layout == DashboardSettings.LAYOUT_COMPACT,
            "invalid settings can never enable or select a layout");
        /* Accepted by config.c: enabled only where enabled=1. */
        String[][] accept = {
            { "version=1\n", "0" }, { "version=1", "0" },
            { "version=1\nenabled=1\nlayout=extended\n", "1" },
            { "version=1\nlayout=extended\n", "0" },
            { "# comment with spaces\n\nversion=1\r\nenabled=1\r\n# x\n", "1" },
            { "version=1\nlayout=compact\nenabled=0\n", "0" },
            { "version=1\nenabled=1", "1" },
        };
        for (int i = 0; i < accept.length; i++)
            check(DashboardConfig.enabled(b(accept[i][0])) == accept[i][1].equals("1"),
                "accept case " + i);
        /* Rejected by config.c; appending enabled=1 must not make any of them enabled. */
        String[] reject = {
            "", "\n", "# only comment\n", "enabled=1\n", "enabled=1\nversion=1\n",
            "version=2\nenabled=1\n", "version=0\n", "version=01\n", "version=1\nversion=1\n",
            "version = 1\n", "version=1 \n", " version=1\n", "\tversion=1\n",
            "version=1\nenabled=1 \n", "version=1\nenabled=2\n", "version=1\nenabled=\n",
            "version=1\nenabled=yes\n", "version=1\nenabled=1\nenabled=1\n",
            "version=1\nenabled=1\nenabled=0\n", "version=1\nlayout=Extended\n",
            "version=1\nlayout=wide\n", "version=1\nlayout=compact\nlayout=compact\n",
            "version=1\nenabled=1\ncolour=red\n", "version=1\nenabled\n",
            "version=1\n=1\n", "version=1\nenabled==1\n", "version=1\n #indented\n",
            "version=1\nenabled=1\nlayout=extended\nx\n", "version=1\nenabled=Ã©\n",
            "version=1\nenabled=1\u007f\n", "version=1\r\r\n",
        };
        for (int i = 0; i < reject.length; i++) {
            check(!DashboardConfig.enabled(b(reject[i])), "reject case " + i);
            check(!DashboardConfig.enabled(b(reject[i] + "\nenabled=1\n")), "reject+enabled case " + i);
        }
        check(!DashboardConfig.enabled(b("version=1\nenabled=1\u0000\n")), "NUL byte");
        check(!DashboardConfig.enabled(b("version=1\nenabled=1\n#\u0000\n")), "NUL in comment");
        check(!DashboardConfig.enabled((byte[]) null), "null text");
        /* Line cap 64 (comments too) and whole-input cap 512. */
        StringBuffer line = new StringBuffer("#");
        while (line.length() < 64) line.append('x');
        check(DashboardConfig.enabled(b("version=1\nenabled=1\n" + line + "\n")), "64-byte line");
        check(!DashboardConfig.enabled(b("version=1\nenabled=1\n" + line + "x\n")), "65-byte line");
        StringBuffer big = new StringBuffer("version=1\nenabled=1\n");
        while (big.length() < 512) big.append('\n');
        check(DashboardConfig.enabled(b(big.toString())), "512 bytes");
        check(!DashboardConfig.enabled(b(big.toString() + "\n")), "513 bytes");

        /* File access through the gate: missing, valid, oversize, disabled. */
        File dir = new File(args[0]);
        File cfg = new File(dir, "carplay_dashboard.cfg");
        cfg.delete();
        DashboardConfig.path = cfg.getPath();
        check(!DashboardConfig.enabled(), "missing file must disable");
        write(cfg, b("version=1\nenabled=1\n"));
        check(DashboardConfig.enabled(), "valid enabled file");
        write(cfg, b(big.toString() + "\n"));
        check(DashboardConfig.read(cfg.getPath()) == null && !DashboardConfig.enabled(), "oversize file");
        write(cfg, b("version=1\nenabled=0\n"));
        check(!DashboardConfig.enabled(), "enabled=0");
        cfg.delete();
        Process fifo = Runtime.getRuntime().exec(new String[] { "mkfifo", cfg.getPath() });
        check(fifo.waitFor() == 0, "create FIFO config");
        check(!DashboardConfig.enabled(), "FIFO config must disable without blocking");
        cfg.delete();
        File target = new File(dir, "real.cfg");
        write(target, b("version=1\nenabled=1\n"));
        Process link = Runtime.getRuntime().exec(new String[] { "ln", "-s", target.getPath(), cfg.getPath() });
        check(link.waitFor() == 0, "create symlink config");
        check(!DashboardConfig.enabled(), "symlink config must disable");
        cfg.delete(); target.delete();
        cfg.mkdir();
        check(!DashboardConfig.enabled(), "directory config must disable");
        check(DashboardConfig.load() == DashboardSettings.DEFAULT, "directory -> default settings");
        cfg.delete();                                   /* remove the directory again */
        write(cfg, b("version=1\nenabled=1\nlayout=extended\n"));
        DashboardSettings ls = DashboardConfig.load();
        check(ls.valid && ls.enabled && ls.layout == DashboardSettings.LAYOUT_EXTENDED, "typed load");
        cfg.delete();
        check(DashboardConfig.load() == DashboardSettings.DEFAULT, "missing file -> default");
        check(DashboardConfig.PATH.equals("/mnt/app/carplay_dashboard.cfg"), "production path");
        System.out.println("DashboardConfigTest: " + nv + " shared vectors, typed settings, config.c parity (accept/reject/caps/NUL), file gate PASS");
    }

    static void write(File f, byte[] data) throws Exception {
        FileOutputStream out = new FileOutputStream(f);
        try { out.write(data); } finally { out.close(); }
    }
}
