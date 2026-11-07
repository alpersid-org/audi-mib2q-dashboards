package com.luka.carplay.core;

/* Dashboard-feed test stub for the stock-linked VehicleDataModule: same
 * DashboardVehicle shape and static accessor; the test controls the source. */
public final class VehicleDataModule {
    public static final class DashboardVehicle {
        public final int tempDc, rangeKm;
        public final long tempMs, rangeMs;
        public DashboardVehicle(int tempDc, long tempMs, int rangeKm, long rangeMs) {
            this.tempDc = tempDc; this.tempMs = tempMs; this.rangeKm = rangeKm; this.rangeMs = rangeMs;
        }
    }
    public interface Source { DashboardVehicle read(); }
    public static volatile Source source;
    public static DashboardVehicle dashboardVehicle() {
        Source s = source;
        return s == null ? null : s.read();
    }
    private VehicleDataModule() { }
}
