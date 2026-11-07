# SPDX-License-Identifier: GPL-3.0-or-later
# Independent host-only module; does not stage an upstream install payload.
CC ?= cc
AR ?= ar
PYTHON ?= python3
CFLAGS ?= -O2 -g
STRICT = -std=c99 -Wall -Wextra -Werror -pedantic
OUT = ../build/dashboard
SOURCES = provider.c mock_provider.c widgets.c layout.c svg.c config.c manager.c vehicle_provider.c
OBJECTS = $(addprefix $(OUT)/,$(SOURCES:.c=.o))
HEADERS = dashboard.h ../maneuver_render/visible_area.h ../maneuver_render/protocol.h
SAN = -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer

.PHONY: all test test-sanitize demo previews benchmark clean
all: $(OUT)/libdashboard.a $(OUT)/dashboard_demo $(OUT)/dashboard_preview \
	$(OUT)/dashboard_config $(OUT)/dashboard_manager_preview

$(OUT):
	mkdir -p $@

$(OUT)/%.o: %.c $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -c $< -o $@

$(OUT)/libdashboard.a: $(OBJECTS)
	$(AR) rcs $@ $^

$(OUT)/dashboard_demo: demo.c $(OUT)/libdashboard.a $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) demo.c $(OUT)/libdashboard.a $(LDFLAGS) -o $@

$(OUT)/dashboard_preview: preview.c $(OUT)/libdashboard.a $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) preview.c $(OUT)/libdashboard.a $(LDFLAGS) -o $@

# P2-03 host tools: config actions and manager-gated mock preview (no install).
$(OUT)/dashboard_config: dashboard_config.c $(OUT)/libdashboard.a $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) dashboard_config.c $(OUT)/libdashboard.a $(LDFLAGS) -o $@

$(OUT)/dashboard_manager_preview: manager_preview.c $(OUT)/libdashboard.a $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) manager_preview.c $(OUT)/libdashboard.a $(LDFLAGS) -o $@

$(OUT)/dashboard_manager_test: ../tests/dashboard_manager_test.c $(SOURCES) $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -UNDEBUG ../tests/dashboard_manager_test.c $(SOURCES) $(LDFLAGS) -o $@

$(OUT)/dashboard_manager_test_sanitize: ../tests/dashboard_manager_test.c $(SOURCES) $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -O1 -UNDEBUG $(SAN) ../tests/dashboard_manager_test.c $(SOURCES) $(LDFLAGS) -o $@

# Bounded host-only measurement; not a head-unit performance claim.
$(OUT)/dashboard_benchmark: benchmark.c $(OUT)/libdashboard.a $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) benchmark.c $(OUT)/libdashboard.a $(LDFLAGS) -o $@

$(OUT)/dashboard_test: ../tests/dashboard_test.c $(SOURCES) $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -UNDEBUG ../tests/dashboard_test.c $(SOURCES) $(LDFLAGS) -o $@

$(OUT)/dashboard_test_sanitize: ../tests/dashboard_test.c $(SOURCES) $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -O1 -UNDEBUG $(SAN) ../tests/dashboard_test.c $(SOURCES) $(LDFLAGS) -o $@

$(OUT)/dashboard_render_test: ../tests/dashboard_render_test.c $(SOURCES) $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -UNDEBUG ../tests/dashboard_render_test.c $(SOURCES) $(LDFLAGS) -o $@

$(OUT)/dashboard_render_test_sanitize: ../tests/dashboard_render_test.c $(SOURCES) $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -O1 -UNDEBUG $(SAN) ../tests/dashboard_render_test.c $(SOURCES) $(LDFLAGS) -o $@

test: $(OUT)/dashboard_test $(OUT)/dashboard_render_test $(OUT)/dashboard_preview \
	$(OUT)/dashboard_manager_test $(OUT)/dashboard_config $(OUT)/dashboard_manager_preview
	$(OUT)/dashboard_test
	$(OUT)/dashboard_render_test $(OUT)/escaped.svg
	$(PYTHON) ../tests/dashboard_svg_test.py $(OUT)/dashboard_preview $(OUT)/escaped.svg
	mkdir -p $(OUT)/fixtures
	$(OUT)/dashboard_manager_test $(OUT)/fixtures
	$(PYTHON) ../tests/dashboard_cli_test.py $(OUT)/dashboard_config $(OUT)/dashboard_manager_preview

test-sanitize: $(OUT)/dashboard_test_sanitize $(OUT)/dashboard_render_test_sanitize \
	$(OUT)/dashboard_manager_test_sanitize
	$(OUT)/dashboard_test_sanitize
	$(OUT)/dashboard_render_test_sanitize $(OUT)/escaped_sanitize.svg
	mkdir -p $(OUT)/fixtures
	$(OUT)/dashboard_manager_test_sanitize $(OUT)/fixtures

benchmark: $(OUT)/dashboard_benchmark
	$(OUT)/dashboard_benchmark

demo: $(OUT)/dashboard_demo
	$(OUT)/dashboard_demo

# Reviewable SVG artifacts only; no installation/staging step.
previews: $(OUT)/dashboard_preview
	mkdir -p previews
	$(OUT)/dashboard_preview compact wide > previews/compact-wide.svg
	$(OUT)/dashboard_preview compact tall > previews/compact-tall.svg
	$(OUT)/dashboard_preview extended wide > previews/extended-wide.svg
	$(OUT)/dashboard_preview extended tall > previews/extended-tall.svg

clean:
	rm -f $(OBJECTS) $(OUT)/libdashboard.a $(OUT)/dashboard_demo $(OUT)/dashboard_test $(OUT)/dashboard_test_sanitize
	rm -f $(OUT)/dashboard_preview $(OUT)/dashboard_render_test $(OUT)/dashboard_render_test_sanitize $(OUT)/escaped.svg $(OUT)/escaped_sanitize.svg
	rm -f $(OUT)/dashboard_config $(OUT)/dashboard_manager_preview $(OUT)/dashboard_benchmark
	rm -f $(OUT)/dashboard_manager_test $(OUT)/dashboard_manager_test_sanitize
	rm -rf $(OUT)/fixtures
