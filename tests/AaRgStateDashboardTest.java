package com.luka.carplay.aa;

/* P2-FINAL: AaRgState.dashboardNav() (P2-08) exposes only the trip data onTrip already
 * resolved (route-start and 30 s freshness guards) plus the destination sample time,
 * and never changes route state.  Real AaRgState; no stubs. */
public final class AaRgStateDashboardTest {
    private static void check(boolean ok, String message) {
        if (!ok) throw new AssertionError(message);
    }

    public static void main(String[] args) throws Exception {
        AaRgState rg = new AaRgState();
        check(rg.dashboardNav() == null, "no route: no dashboard navigation");
        check(rg.onTurn("Main St", 1, AaManeuverMap.EV_TURN, 0, 0, 1), "route start");
        AaRgState.DashboardNav n = rg.dashboardNav();
        check(n != null && n.destination.equals("") && n.meters == -1 && n.seconds == -1
            && n.destinationMs == 0 && n.estimateMs == 0, "route without trip data");
        int gen = (int) rg.routeGeneration();

        long now = System.currentTimeMillis();
        rg.onTrip(new AaTripFeed.Trip("City Centre", 42500, 2280, now - 10, now - 5), now, now);
        n = rg.dashboardNav();
        check(n.destination.equals("City Centre") && n.meters == 42500 && n.seconds == 2280, "values");
        check(n.destinationMs == now - 10 && n.estimateMs == now - 5, "sample times");

        /* Stale estimate (> 30 s): dropped; fresh destination kept with its own time. */
        rg.onTrip(new AaTripFeed.Trip("City Centre", 1, 1, now - 20, now - 40000), now, now);
        n = rg.dashboardNav();
        check(n.meters == -1 && n.seconds == -1 && n.estimateMs == 0, "stale estimate dropped");
        check(n.destination.equals("City Centre") && n.destinationMs == now - 20, "destination kept");

        /* Stale destination: name and its time dropped. */
        rg.onTrip(new AaTripFeed.Trip("Old", 7, 8, now - 40000, now - 3), now, now);
        n = rg.dashboardNav();
        check(n.destination.equals("") && n.destinationMs == 0 && n.meters == 7 && n.estimateMs == now - 3,
            "stale destination dropped");

        check(rg.routeGeneration() == gen && rg.isRouteActive(), "dashboardNav changed route state");

        /* Route end clears; a sample from before the new route is not revived. */
        rg.onTurn("", 3, 0, 0, 0, AaRgState.NO_GUIDANCE);
        check(!rg.isRouteActive() && rg.dashboardNav() == null, "route end");
        long before = System.currentTimeMillis() - 1;
        rg.onTurn("Second St", 2, AaManeuverMap.EV_TURN, 0, 0, 1);
        rg.onTrip(new AaTripFeed.Trip("Previous", 9, 9, before - 5, before - 5), before + 2, before + 2);
        n = rg.dashboardNav();
        check(n != null && n.destination.equals("") && n.estimateMs == 0, "old trip revived after route end");
        rg.resetSession();
        check(rg.dashboardNav() == null, "session reset");
        System.out.println("AaRgStateDashboardTest: route gating, values/times, 30 s freshness, no state change, route end/session reset PASS");
    }
}
