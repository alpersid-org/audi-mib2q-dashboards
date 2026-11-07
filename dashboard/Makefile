# SPDX-License-Identifier: GPL-3.0-or-later
# Independent host-only module; does not stage an upstream install payload.
CC ?= cc
AR ?= ar
CFLAGS ?= -O2 -g
STRICT = -std=c99 -Wall -Wextra -Werror -pedantic
OUT = ../build/dashboard
SOURCES = provider.c mock_provider.c widgets.c
OBJECTS = $(addprefix $(OUT)/,$(SOURCES:.c=.o))
HEADERS = dashboard.h ../maneuver_render/visible_area.h ../maneuver_render/protocol.h
SAN = -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer

.PHONY: all test test-sanitize demo clean
all: $(OUT)/libdashboard.a $(OUT)/dashboard_demo

$(OUT):
	mkdir -p $@

$(OUT)/%.o: %.c $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -c $< -o $@

$(OUT)/libdashboard.a: $(OBJECTS)
	$(AR) rcs $@ $^

$(OUT)/dashboard_demo: demo.c $(OUT)/libdashboard.a $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) demo.c $(OUT)/libdashboard.a $(LDFLAGS) -o $@

$(OUT)/dashboard_test: ../tests/dashboard_test.c $(SOURCES) $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -UNDEBUG ../tests/dashboard_test.c $(SOURCES) $(LDFLAGS) -o $@

$(OUT)/dashboard_test_sanitize: ../tests/dashboard_test.c $(SOURCES) $(HEADERS) | $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(STRICT) -O1 -UNDEBUG $(SAN) ../tests/dashboard_test.c $(SOURCES) $(LDFLAGS) -o $@

test: $(OUT)/dashboard_test
	$(OUT)/dashboard_test

test-sanitize: $(OUT)/dashboard_test_sanitize
	$(OUT)/dashboard_test_sanitize

demo: $(OUT)/dashboard_demo
	$(OUT)/dashboard_demo

clean:
	rm -f $(OBJECTS) $(OUT)/libdashboard.a $(OUT)/dashboard_demo $(OUT)/dashboard_test $(OUT)/dashboard_test_sanitize
