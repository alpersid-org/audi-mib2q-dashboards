package com.luka.carplay.aa;

/* Dashboard-feed test stub for the stock-linked AaBridge: returns a test-set value of
 * the REAL AaRgState.DashboardNav type (AaRgState itself is compiled from java_patch). */
public final class AaBridge {
    public static volatile AaRgState.DashboardNav nav;
    public static AaRgState.DashboardNav dashboardNav() { return nav; }
    public static AaRgState.DashboardNav make(String d, int m, int s, long dMs, long eMs) {
        return new AaRgState.DashboardNav(d, m, s, dMs, eMs);
    }
    private AaBridge() { }
}
