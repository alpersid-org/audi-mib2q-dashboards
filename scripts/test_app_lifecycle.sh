#!/bin/bash
# Isolated CarPlayApp lifecycle test (same compile as the lifecycle block of
# test_java_transports.sh). Needs only a host JDK: no stock JAR, no shipping build.
# Added in P2-FINAL so the R4 harness repair can run where stock assets are absent.
set -euo pipefail
PROJECT_DIR=$(cd "$(dirname "$0")/.." && pwd)
JAVAC=${JAVAC:-javac}
JAVA=${JAVA:-java}
TEST_DIR=$(mktemp -d)
trap 'rm -rf "$TEST_DIR"' EXIT
S="$PROJECT_DIR/tests/stubs/app-lifecycle"
"$JAVAC" -encoding UTF-8 -d "$TEST_DIR" \
    "$PROJECT_DIR/java_patch/com/luka/carplay/core/CarPlayApp.java" \
    "$PROJECT_DIR/java_patch/com/luka/carplay/core/Module.java" \
    "$PROJECT_DIR/tests/CarPlayAppLifecycleTest.java" \
    "$S/com/luka/carplay/core/LifecycleFixtures.java" \
    "$S/com/luka/carplay/core/FrameworkRef.java" \
    "$S/com/luka/carplay/dashboard/DashboardModule.java" \
    "$S/com/luka/carplay/bus/CarplayBus.java" \
    "$S/com/luka/carplay/framework/Log.java" \
    "$S/com/luka/carplay/pdc/PdcSmallStageGuard.java" \
    "$S/de/audi/app/terminalmode/IContext.java" \
    "$S/de/audi/atip/base/IFrameworkAccess.java"
for scenario in publication during-start replug failure bounce owners; do
    printf '%-14s ' "$scenario"
    "$JAVA" -cp "$TEST_DIR" com.luka.carplay.core.CarPlayAppLifecycleTest "$scenario"
done
