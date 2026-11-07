package com.luka.carplay.core;

/* Dashboard-feed test stub: only the session-owner API DashboardModule uses. */
public final class CarPlayApp {
    public static final int OWNER_NONE = 0, OWNER_CARPLAY = 1, OWNER_ANDROID_AUTO = 2;
    public static volatile int owner = OWNER_NONE;
    public static int sessionOwner() { return owner; }
    private CarPlayApp() { }
}
