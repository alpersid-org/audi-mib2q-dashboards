/* SPDX-License-Identifier: GPL-3.0-or-later */
package com.luka.carplay.dashboard;

import com.luka.carplay.aa.AaBridge;
import com.luka.carplay.aa.AaRgState;
import com.luka.carplay.core.CarPlayApp;
import com.luka.carplay.core.FrameworkRef;
import com.luka.carplay.framework.Log;
import com.luka.carplay.core.Module;
import com.luka.carplay.core.VehicleDataModule;
import java.io.File;
import java.io.FileOutputStream;

/**
 * P2-08 VehicleDataProvider, HMI side.  While the custom dashboard is enabled
 * (DashboardConfig), publishes the vehicle data the HMI already receives as a fixed
 * record for the dashboard renderer, every PERIOD_MS:
 *
 *   outside temperature, range   VehicleDataModule (DSICarKombi, existing bindings)
 *   final-destination distance,  AaRgState via AaBridge (Android Auto sessions only;
 *   time and name                  already route-start and freshness guarded)
 *
 * Media is not published: during Android Auto it is only reachable through stock
 * DSIAndroidAuto2 classes whose signatures are not available here (scope req. 8).
 * No new DSI attributes, sockets or renderer commands.  Disabled: does nothing and
 * removes any old record, so a reader finds no data.
 *
 * DBV1 record, 256 bytes, big-endian (C reader: dashboard/vehicle_provider.c):
 *   [0]   magic "DBV1"            [4]  sequence u32         [8]  written ms u64
 *   [16]  temperature 0.1 C i32 (INT_MIN = none)            [20] range m i32 (-1 = none)
 *   [24]  destination m i32 (-1)  [28] destination s i32 (-1)
 *   [32]  temperature ms u64      [40] range ms u64         [48] navigation ms u64
 *   [56]  session owner u8 (0 none, 1 CarPlay, 2 Android Auto), [57..59] zero
 *   [60]  writer session u32 (non-zero; new for every start and HMI process)
 *   [64..191] destination UTF-8, NUL-terminated, at most 95 bytes
 *   [192..251] zero               [252] FNV-1a u32 over [0..251]
 * All times are System.currentTimeMillis() sample times; 0 = that part is absent.
 * The navigation time is the OLDEST contributing sample (destination name, estimate).
 * The write time is taken AFTER all sources were read, so no sample is newer than it
 * (review R1).  Sequence starts at 1 for each writer session and increases by one.
 *
 * Lifecycle (review R2): every start() opens a new writer generation. At most one
 * publication stages its data at a time. File open/write/close run outside the
 * coordination lock; the final generation check and rename run under it. stop()
 * invalidates the generation and deletes the published record under that same lock.
 * A blocked or cancelled old writer therefore cannot commit after stop, or replace a
 * newer generation's record. stop() does not wait for bulk file I/O. Temporary files
 * are per generation and cleaned by their owning writer.
 *
 * Pure Java 1.4 apart from the module interfaces.
 */
public final class DashboardModule implements Module {
    private static final String TAG = "Dashboard";
    public static final String PATH = "/tmp/carplay_dashboard";
    static final int RECORD_SIZE = 256, DEST_OFF = 64, DEST_MAX = 95;
    static final int MAGIC = 0x44425631;
    private static final long PERIOD_MS = 500L;
    /* Host tests point this at a temporary directory; production always uses PATH. */
    static volatile String recordPath = PATH;

    /* Distinguishes HMI process starts; mixed with the generation into the session id. */
    private static final int BOOT = (int)(System.currentTimeMillis() ^ (System.currentTimeMillis() >>> 32));

    private Thread writer;                      /* guarded by this */
    private boolean running;                    /* guarded by this */

    /* Publication coordination (review R2). Bulk file I/O stays outside; the small
     * rename/delete metadata operations must be ordered with generation changes. */
    private final Object files = new Object();
    private int generation;                     /* guarded by files */
    private boolean publishing;                 /* guarded by files */

    public String name() { return "dashboard"; }

    public synchronized boolean start(FrameworkRef fw) {
        if (running) return true;
        if (!DashboardConfig.enabled()) {
            synchronized (files) {
                generation++;                       /* old writers cannot commit */
                removeRecord();
            }
            return true;                       /* disabled by default: nothing to run */
        }
        final int gen;
        synchronized (files) { gen = ++generation; }
        running = true;
        writer = new Thread(new Runnable() {
            public void run() { writeLoop(gen); }
        }, "carplay-dashboard");
        writer.setDaemon(true);
        writer.start();
        Log.i(TAG, "dashboard feed enabled: " + recordPath + " generation " + gen);
        return true;
    }

    public synchronized void stop() {
        running = false;
        Thread t = writer;
        writer = null;
        if (t != null) t.interrupt();
        synchronized (files) {
            generation++;
            /* Clear the last successful publication even if staging I/O is blocked. */
            removeRecord();
        }
    }

    private boolean current(int gen) {
        synchronized (files) { return generation == gen; }
    }

    static int sessionId(int gen) {
        int id = BOOT ^ (gen * 0x9e3779b9);         /* bijective in gen for one process */
        return id != 0 ? id : 1;
    }

    private void writeLoop(int gen) {
        boolean failLogged = false;
        int sequence = 0;
        int session = sessionId(gen);
        while (current(gen)) {
            try { Thread.sleep(PERIOD_MS); }
            catch (InterruptedException e) { return; }
            if (!current(gen)) return;
            try {
                /* Read every source first, then take the write time (review R1). */
                int owner = CarPlayApp.sessionOwner();
                AaRgState.DashboardNav nav =
                    owner == CarPlayApp.OWNER_ANDROID_AUTO ? AaBridge.dashboardNav() : null;
                VehicleDataModule.DashboardVehicle vehicle = VehicleDataModule.dashboardVehicle();
                long written = System.currentTimeMillis();
                byte[] rec = encode(++sequence, written, owner, session, vehicle, nav);
                int result = publish(gen, rec);
                if (result < 0) return;                 /* generation ended */
                if (result == 0) {
                    if (!failLogged) { failLogged = true; Log.w(TAG, "record write failed: " + recordPath); }
                } else {
                    failLogged = false;
                }
            } catch (Throwable t) {
                if (!failLogged) { failLogged = true; Log.w(TAG, "dashboard feed: " + t); }
            }
        }
    }

    /** 1 published, 0 failed or skipped (try again), -1 the generation has ended. */
    private int publish(int gen, byte[] rec) {
        synchronized (files) {
            if (generation != gen) return -1;
            if (publishing) return 0;              /* an older writer is still finishing */
            publishing = true;
        }
        boolean ok = false;
        try {
            ok = writeAndRename(rec, gen);
        } finally {
            synchronized (files) {
                publishing = false;
                if (generation != gen) ok = false;
            }
        }
        return ok ? 1 : (current(gen) ? 0 : -1);
    }

    static byte[] encode(int seq, long now, int owner, int session,
                         VehicleDataModule.DashboardVehicle v,
                         AaRgState.DashboardNav nav) throws Exception {
        byte[] b = new byte[RECORD_SIZE];
        putInt(b, 0, MAGIC);
        putInt(b, 4, seq);
        putLong(b, 8, now);
        int temp = Integer.MIN_VALUE, range = -1;
        long tempMs = 0, rangeMs = 0;
        if (v != null) {
            if (v.tempMs > 0 && v.tempDc != Integer.MIN_VALUE) { temp = v.tempDc; tempMs = v.tempMs; }
            if (v.rangeMs > 0 && v.rangeKm >= 0) { range = v.rangeKm * 1000; rangeMs = v.rangeMs; }
        }
        int meters = -1, seconds = -1;
        long navMs = 0;
        byte[] dest = new byte[0];
        if (nav != null) {
            boolean hasEstimate = nav.estimateMs > 0 && (nav.meters >= 0 || nav.seconds >= 0);
            boolean hasName = nav.destinationMs > 0 && nav.destination.length() > 0;
            if (hasEstimate) { meters = nav.meters; seconds = nav.seconds; }
            if (hasName) dest = truncateUtf8(nav.destination.getBytes("UTF-8"), DEST_MAX);
            if (hasEstimate && hasName) navMs = Math.min(nav.estimateMs, nav.destinationMs);
            else if (hasEstimate) navMs = nav.estimateMs;
            else if (hasName) navMs = nav.destinationMs;
        }
        putInt(b, 16, temp);
        putInt(b, 20, range);
        putInt(b, 24, meters);
        putInt(b, 28, seconds);
        putLong(b, 32, tempMs);
        putLong(b, 40, rangeMs);
        putLong(b, 48, navMs);
        b[56] = (byte)(owner == CarPlayApp.OWNER_CARPLAY ? 1
                     : owner == CarPlayApp.OWNER_ANDROID_AUTO ? 2 : 0);
        putInt(b, 60, session);
        System.arraycopy(dest, 0, b, DEST_OFF, dest.length);   /* NUL follows: array is zeroed */
        int hash = 0x811c9dc5;
        for (int i = 0; i < RECORD_SIZE - 4; i++) hash = (hash ^ (b[i] & 255)) * 16777619;
        putInt(b, RECORD_SIZE - 4, hash);
        return b;
    }

    /* Cut at max bytes without splitting a UTF-8 sequence. */
    static byte[] truncateUtf8(byte[] s, int max) {
        if (s.length <= max) return s;
        int n = max;
        while (n > 0 && (s[n] & 0xc0) == 0x80) n--;      /* s[n] is the first dropped byte */
        byte[] out = new byte[n];
        System.arraycopy(s, 0, out, 0, n);
        return out;
    }

    /* Stage outside the lock; commit only while this generation is still current.
     * POSIX rename replaces atomically; the fallback may briefly leave no record. */
    private boolean writeAndRename(byte[] rec, int gen) {
        File tmp = new File(recordPath + ".tmp." + gen), dst = new File(recordPath);
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(tmp);
            out.write(rec);
            out.close();
            out = null;
            synchronized (files) {
                if (generation != gen) return false;
                if (tmp.renameTo(dst)) return true;
                dst.delete();
                return tmp.renameTo(dst);
            }
        } catch (Throwable t) {
            return false;
        } finally {
            if (out != null) try { out.close(); } catch (Throwable t) { }
            try { tmp.delete(); } catch (Throwable t) { }
        }
    }

    /* Remove only the published record (call with files held). In-flight writers
     * own their temporary files, including a blocked open that stop must not await. */
    private static void removeRecord() {
        try { new File(recordPath).delete(); } catch (Throwable t) { }
    }

    private static void putInt(byte[] b, int p, int v) {
        b[p] = (byte)(v >>> 24); b[p + 1] = (byte)(v >>> 16); b[p + 2] = (byte)(v >>> 8); b[p + 3] = (byte)v;
    }

    private static void putLong(byte[] b, int p, long v) {
        putInt(b, p, (int)(v >>> 32));
        putInt(b, p + 4, (int)v);
    }
}
