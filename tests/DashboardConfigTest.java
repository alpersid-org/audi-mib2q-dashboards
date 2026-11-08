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

    public static void main(String[] args) throws Exception {
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
        cfg.delete();
        check(DashboardConfig.PATH.equals("/mnt/app/carplay_dashboard.cfg"), "production path");
        System.out.println("DashboardConfigTest: config.c parity (accept/reject/caps/NUL), file gate PASS");
    }

    static void write(File f, byte[] data) throws Exception {
        FileOutputStream out = new FileOutputStream(f);
        try { out.write(data); } finally { out.close(); }
    }
}
