package com.luka.carplay.dashboard;

import com.luka.carplay.aa.AaBridge;
import com.luka.carplay.core.CarPlayApp;
import com.luka.carplay.core.VehicleDataModule;
import java.io.File;
import java.io.FileOutputStream;
import java.io.RandomAccessFile;

/* P2-FINAL tests for the real DashboardModule (P2-08 feed, review fixes R1/R2 and the
 * writer session).  VehicleDataModule/AaBridge/CarPlayApp are stubs
 * (tests/stubs/dashboard-feed); AaRgState.DashboardNav is the real class.
 * Usage: DashboardModuleTest <scratch dir> <fixture output for the C test> */
public final class DashboardModuleTest {
    static final long T = 1700000000000L;

    private static void check(boolean ok, String message) {
        if (!ok) throw new AssertionError(message);
    }
    static int u32(byte[] b, int p) {
        return (b[p] & 255) << 24 | (b[p + 1] & 255) << 16 | (b[p + 2] & 255) << 8 | (b[p + 3] & 255);
    }
    static long u64(byte[] b, int p) { return ((long) u32(b, p) << 32) | (u32(b, p + 4) & 0xffffffffL); }
    static boolean zero(byte[] b, int from, int to) {
        for (int i = from; i < to; i++) if (b[i] != 0) return false;
        return true;
    }
    static void checksum(byte[] b) {
        int h = 0x811c9dc5;
        for (int i = 0; i < 252; i++) h = (h ^ (b[i] & 255)) * 16777619;
        check(h == u32(b, 252), "checksum");
    }

    static void layout() throws Exception {
        VehicleDataModule.DashboardVehicle v = new VehicleDataModule.DashboardVehicle(-35, T + 10, 620, T + 20);
        byte[] b = DashboardModule.encode(3, T + 50, CarPlayApp.OWNER_ANDROID_AUTO, 0x01020304, v,
            AaBridge.make("City", 42500, 2280, T + 30, T + 40));
        check(b.length == 256 && u32(b, 0) == 0x44425631 && u32(b, 4) == 3 && u64(b, 8) == T + 50, "header");
        check(u32(b, 16) == -35 && u32(b, 20) == 620000, "temperature/range (km -> m)");
        check(u32(b, 24) == 42500 && u32(b, 28) == 2280, "estimate");
        check(u64(b, 32) == T + 10 && u64(b, 40) == T + 20, "vehicle times");
        check(u64(b, 48) == T + 30, "navigation time is the oldest part");
        check(b[56] == 2 && zero(b, 57, 60) && u32(b, 60) == 0x01020304, "owner/session");
        check(new String(b, 64, 4, "UTF-8").equals("City") && zero(b, 68, 252), "destination/padding");
        checksum(b);
        /* Absent parts and CarPlay owner (no navigation in CarPlay sessions). */
        b = DashboardModule.encode(1, T, CarPlayApp.OWNER_CARPLAY, 9,
            new VehicleDataModule.DashboardVehicle(Integer.MIN_VALUE, 0, -1, 0), null);
        check(u32(b, 16) == Integer.MIN_VALUE && u32(b, 20) == -1 && u64(b, 32) == 0 && u64(b, 40) == 0, "absent vehicle");
        check(u32(b, 24) == -1 && u32(b, 28) == -1 && u64(b, 48) == 0 && b[56] == 1 && b[64] == 0, "absent nav");
        checksum(b);
        b = DashboardModule.encode(1, T, CarPlayApp.OWNER_NONE, 9, null, null);
        check(b[56] == 0 && u64(b, 32) == 0, "no session / no vehicle");
        /* Navigation parts: estimate only, name only; a stale (0-time) part is dropped. */
        b = DashboardModule.encode(1, T, 2, 9, null, AaBridge.make("", 100, -1, 0, T - 5));
        check(u64(b, 48) == T - 5 && u32(b, 24) == 100 && u32(b, 28) == -1 && b[64] == 0, "estimate only");
        b = DashboardModule.encode(1, T, 2, 9, null, AaBridge.make("X", -1, -1, T - 7, 0));
        check(u64(b, 48) == T - 7 && u32(b, 24) == -1 && b[64] == 'X', "name only");
        b = DashboardModule.encode(1, T, 2, 9, null, AaBridge.make("X", 100, 5, 0, 0));
        check(u64(b, 48) == 0 && b[64] == 0 && u32(b, 24) == -1, "parts without times are absent");
        /* UTF-8 truncation at 95 bytes without splitting a character. */
        StringBuffer s = new StringBuffer();
        for (int i = 0; i < 50; i++) s.append('ä');            /* 2 bytes each */
        b = DashboardModule.encode(1, T, 2, 9, null, AaBridge.make(s.toString(), 1, 1, T, T));
        int n = 0;
        while (b[64 + n] != 0) n++;
        check(n == 94 && new String(b, 64, n, "UTF-8").length() == 47, "UTF-8 cut on boundary");
        check(DashboardModule.truncateUtf8("abc".getBytes("UTF-8"), 95).length == 3, "short kept");
        check(DashboardModule.sessionId(1) != 0 && DashboardModule.sessionId(1) != DashboardModule.sessionId(2),
            "session id per generation");
    }

    /* Fixed record for tests/dashboard_vehicle_test.c (values asserted there). */
    static void writeFixture(String path) throws Exception {
        byte[] b = DashboardModule.encode(7, T + 500, CarPlayApp.OWNER_ANDROID_AUTO, 0x12345678,
            new VehicleDataModule.DashboardVehicle(125, T + 400, 620, T + 300),
            AaBridge.make("Zürich Hbf", 42500, 2280, T + 200, T + 100));
        FileOutputStream out = new FileOutputStream(path);
        try { out.write(b); } finally { out.close(); }
    }

    static byte[] read(File f) {
        RandomAccessFile r = null;
        try {
            r = new RandomAccessFile(f, "r");
            if (r.length() != 256) return null;
            byte[] b = new byte[256];
            r.readFully(b);
            return b;
        } catch (Exception e) { return null; }
        finally { if (r != null) try { r.close(); } catch (Exception e) { } }
    }

    static byte[] waitRecord(File f, long ms) throws Exception {
        long end = System.currentTimeMillis() + ms;
        while (System.currentTimeMillis() < end) {
            byte[] b = read(f);
            if (b != null) return b;
            Thread.sleep(20);
        }
        return null;
    }

    static void absentFor(File f, long ms, String what) throws Exception {
        long end = System.currentTimeMillis() + ms;
        while (System.currentTimeMillis() < end) {
            check(!f.exists(), what);
            Thread.sleep(20);
        }
    }

    static void lifecycle(File dir) throws Exception {
        File cfg = new File(dir, "carplay_dashboard.cfg"), rec = new File(dir, "carplay_dashboard");
        DashboardConfig.path = cfg.getPath();
        DashboardModule.recordPath = rec.getPath();
        DashboardConfigTest.write(cfg, "version=1\nenabled=1\n".getBytes("ISO-8859-1"));
        CarPlayApp.owner = CarPlayApp.OWNER_ANDROID_AUTO;
        AaBridge.nav = null;

        /* R1: a vehicle read that takes time and stamps its samples at the end. */
        VehicleDataModule.source = new VehicleDataModule.Source() {
            public VehicleDataModule.DashboardVehicle read() {
                try { Thread.sleep(3); } catch (InterruptedException e) { }
                long now = System.currentTimeMillis();
                return new VehicleDataModule.DashboardVehicle(125, now, 620, now);
            }
        };
        DashboardModule m = new DashboardModule();
        check(m.start(null), "start");
        byte[] b = waitRecord(rec, 3000);
        check(b != null, "no record published");
        int session = u32(b, 60), seq = u32(b, 4);
        check(session != 0 && seq >= 1, "session/sequence");
        for (int i = 0; i < 4; i++) {
            Thread.sleep(520);
            b = read(rec);
            check(b != null, "record missing while running");
            check(u64(b, 32) <= u64(b, 8) && u64(b, 40) <= u64(b, 8), "R1: sample newer than write time");
            check(u32(b, 60) == session && u32(b, 4) > seq, "sequence must advance in one session");
            seq = u32(b, 4);
        }
        check(new File(rec.getPath() + ".tmp.1").exists() == false, "temp file left behind");

        /* R2: stop removes the record and nothing is published afterwards. */
        m.stop();
        check(!rec.exists(), "record survived stop");
        absentFor(rec, 1300, "late publication after stop");

        /* Immediate restart: only the new generation's session appears. */
        check(m.start(null), "restart");
        b = waitRecord(rec, 3000);
        check(b != null && u32(b, 60) != session, "restart must use a new writer session");
        int session2 = u32(b, 60);
        m.stop(); m.start(null);
        b = waitRecord(rec, 3000);
        check(b != null && u32(b, 60) != session2 && u32(b, 60) != session, "third session");
        int session3 = u32(b, 60);
        for (int i = 0; i < 3; i++) {
            Thread.sleep(400);
            b = read(rec);
            check(b != null && u32(b, 60) == session3, "old generation replaced the new record");
        }
        m.stop();
        check(!rec.exists(), "record survived second stop");

        /* R2: stop while the writer is inside the vehicle read (collection phase). */
        final Object gate = new Object();
        final boolean[] state = new boolean[2];             /* [0] entered, [1] released */
        VehicleDataModule.source = new VehicleDataModule.Source() {
            public VehicleDataModule.DashboardVehicle read() {
                synchronized (gate) {
                    state[0] = true; gate.notifyAll();
                    long end = System.currentTimeMillis() + 5000;
                    while (!state[1] && System.currentTimeMillis() < end)
                        try { gate.wait(50); } catch (InterruptedException e) { }
                }
                long now = System.currentTimeMillis();
                return new VehicleDataModule.DashboardVehicle(125, now, 620, now);
            }
        };
        DashboardModule c = new DashboardModule();
        c.start(null);
        synchronized (gate) {
            long end = System.currentTimeMillis() + 3000;
            while (!state[0] && System.currentTimeMillis() < end) gate.wait(50);
        }
        check(state[0], "writer never entered collection");
        long t0 = System.currentTimeMillis();
        c.stop();
        check(System.currentTimeMillis() - t0 < 200, "stop waited for the collecting writer");
        synchronized (gate) { state[1] = true; gate.notifyAll(); }
        absentFor(rec, 1300, "publication after stop during collection");

        /* Stop during a permanently blocked second stage: restart must publish
         * before the old I/O is released; its eventual completion cannot commit. */
        VehicleDataModule.source = null;
        final Object ioGate = new Object();
        final boolean[] ioState = new boolean[2];
        final int[] writes = new int[1];
        DashboardModule w = new DashboardModule(new DashboardModule.RecordOutput() {
            public void write(File file, byte[] record) throws Exception {
                synchronized (ioGate) {
                    if (++writes[0] == 2) {
                        ioState[0] = true; ioGate.notifyAll();
                        while (!ioState[1]) {
                            try { ioGate.wait(); } catch (InterruptedException ignored) { }
                        }
                    }
                }
                FileOutputStream out = new FileOutputStream(file);
                try { out.write(record); } finally { out.close(); }
            }
        });
        w.start(null);
        b = waitRecord(rec, 3000);
        check(b != null, "no earlier record before blocked write");
        int blockedSession = u32(b, 60);
        synchronized (ioGate) {
            long end = System.currentTimeMillis() + 3000;
            while (!ioState[0] && System.currentTimeMillis() < end) ioGate.wait(50);
        }
        check(ioState[0] && rec.exists(), "second write did not block after publication");
        t0 = System.currentTimeMillis();
        w.stop();
        check(System.currentTimeMillis() - t0 < 200, "stop waited for blocked file I/O");
        check(!rec.exists(), "earlier published record survived stop during blocked write");
        w.start(null);
        b = waitRecord(rec, 3000);  // old I/O is STILL blocked
        check(b != null && u32(b, 60) != blockedSession, "old I/O prevented restart publication");
        int recoveredSession = u32(b, 60);
        synchronized (ioGate) { ioState[1] = true; ioGate.notifyAll(); }
        for (int i = 0; i < 4; i++) {
            Thread.sleep(300);
            b = read(rec);
            check(b != null && u32(b, 60) == recoveredSession, "old writer replaced restart");
        }
        w.stop();
        absentFor(rec, 700, "in-flight write left a record after stop");
        String[] files = dir.list();
        for (int i = 0; i < files.length; i++)
            check(!files[i].startsWith("carplay-dashboard-"), "staged file left after completion");

        /* Disabled config: start removes a stale record and publishes nothing. */
        DashboardConfigTest.write(cfg, "version=1\nenabled=0\n".getBytes("ISO-8859-1"));
        DashboardConfigTest.write(rec, new byte[256]);
        DashboardModule d = new DashboardModule();
        check(d.start(null), "disabled start");
        check(!rec.exists(), "disabled start kept a stale record");
        absentFor(rec, 1100, "disabled module published");
        d.stop();
        cfg.delete();
        check(!new DashboardModule().start(null) || !rec.exists(), "missing config");
        absentFor(rec, 700, "missing config published");
    }

    public static void main(String[] args) throws Exception {
        layout();
        writeFixture(args[1]);
        lifecycle(new File(args[0]));
        check(DashboardModule.PATH.equals("/tmp/carplay_dashboard"), "production record path");
        System.out.println("DashboardModuleTest: DBV1 layout/oldest time/UTF-8 cut, R1 write-time order, R2 stop during collection and blocked write, restart sessions, disabled/missing config PASS");
    }
}
