#!/bin/bash
# P2-FINAL: Java tests for the P2-08 dashboard feed with a host JDK (no stock JAR).
#   DashboardConfigTest      real DashboardConfig (config.c parity, file gate)
#   AaRgStateDashboardTest   real AaRgState/AaTripFeed/AaManeuverMap/AaLaneFeed
#   DashboardModuleTest      real DashboardModule + DashboardConfig; stubs for the
#                            stock-linked VehicleDataModule/AaBridge/CarPlayApp
# Then the C reader parses the Java-encoded record (cross-language fixture).
# Usage: test_dashboard_java.sh <build dir containing dashboard_vehicle_test>
set -euo pipefail
PROJECT_DIR=$(cd "$(dirname "$0")/.." && pwd)
OUT=$(cd "$1" && pwd)
JAVAC=${JAVAC:-javac}
JAVA=${JAVA:-java}
TEST_DIR=$(mktemp -d)
trap 'rm -rf "$TEST_DIR"' EXIT
J="$PROJECT_DIR/java_patch/com/luka/carplay"
S="$PROJECT_DIR/tests/stubs/dashboard-feed/com/luka/carplay"
mkdir -p "$TEST_DIR/classes" "$TEST_DIR/scratch"
"$JAVAC" -encoding UTF-8 -d "$TEST_DIR/classes" \
    "$J/dashboard/DashboardConfig.java" "$J/dashboard/DashboardSettings.java" \
    "$J/dashboard/DashboardModule.java" \
    "$J/aa/AaRgState.java" "$J/aa/AaTripFeed.java" "$J/aa/AaManeuverMap.java" "$J/aa/AaLaneFeed.java" \
    "$J/core/Module.java" \
    "$S/core/CarPlayApp.java" "$S/core/FrameworkRef.java" "$S/core/VehicleDataModule.java" \
    "$S/aa/AaBridge.java" \
    "$PROJECT_DIR/tests/stubs/app-lifecycle/com/luka/carplay/framework/Log.java" \
    "$PROJECT_DIR/tests/DashboardConfigTest.java" "$PROJECT_DIR/tests/DashboardModuleTest.java" \
    "$PROJECT_DIR/tests/AaRgStateDashboardTest.java"
"$JAVA" -cp "$TEST_DIR/classes" com.luka.carplay.dashboard.DashboardConfigTest "$TEST_DIR/scratch" \
    "$PROJECT_DIR/tests/fixtures/dashboard_config_vectors.txt"
"$JAVA" -cp "$TEST_DIR/classes" com.luka.carplay.aa.AaRgStateDashboardTest
"$JAVA" -cp "$TEST_DIR/classes" com.luka.carplay.dashboard.DashboardModuleTest \
    "$TEST_DIR/scratch" "$TEST_DIR/java_record.bin"
"$OUT/dashboard_vehicle_test" "$TEST_DIR/scratch" "$TEST_DIR/java_record.bin"
