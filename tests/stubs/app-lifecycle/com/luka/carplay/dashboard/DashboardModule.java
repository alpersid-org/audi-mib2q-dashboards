package com.luka.carplay.dashboard;

import com.luka.carplay.core.CarPlayApp;
import com.luka.carplay.core.FrameworkRef;
import com.luka.carplay.core.Module;

/* Lifecycle fixture for the P2-08 DashboardModule (review R4 repair).  Counts the
 * starts/stops CarPlayApp performs and, like LifecycleTestModule, fails if it is called
 * while CarPlayApp's state lock is held.  The real module's behaviour is covered by
 * DashboardModuleTest. */
public final class DashboardModule implements Module {
    public static final Object gate = new Object();
    public static int starts, stops;
    public static volatile boolean badLockOrder;

    private static void checkLock() {
        try {
            java.lang.reflect.Field field = CarPlayApp.class.getDeclaredField("lock");
            field.setAccessible(true);
            if (Thread.holdsLock(field.get(null))) {
                badLockOrder = true;
                throw new AssertionError("dashboard module called under state lock");
            }
        } catch (NoSuchFieldException e) { throw new AssertionError(e); }
        catch (IllegalAccessException e) { throw new AssertionError(e); }
    }

    public String name() { return "dashboard"; }
    public boolean start(FrameworkRef fw) { checkLock(); synchronized (gate) { starts++; } return true; }
    public void stop() { checkLock(); synchronized (gate) { stops++; } }
    public static int startCount() { synchronized (gate) { return starts; } }
}
