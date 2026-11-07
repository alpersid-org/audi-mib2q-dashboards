package com.luka.carplay.core;

import de.audi.app.terminalmode.IContext;
import de.audi.atip.base.IFrameworkAccess;

/* Fake framework reference for the isolated lifecycle test (was in LifecycleFixtures).
 * Public because modules in other packages (dashboard) implement Module.start(FrameworkRef). */
public final class FrameworkRef {
    static volatile boolean failNext;
    FrameworkRef(IContext context) {
        if (failNext) { failNext = false; throw new IllegalStateException("injected framework failure"); }
    }
    boolean isReady() { return true; }
    IFrameworkAccess framework() { return null; }
}
