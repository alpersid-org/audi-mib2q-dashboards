/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-04 tests for P2-03 configuration and dashboard manager (host only). */
#include "../dashboard/dashboard.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const cr_rect_t WIDE = {0, 0, 800, 252};

static int is_default(const db_config *c)
{
    return c->version == DB_CONFIG_VERSION && c->enabled == 0 && c->layout == DB_COMPACT;
}

static int parse(const char *text, db_config *c)
{
    memset(c, 0x5a, sizeof(*c)); /* garbage must never survive */
    return db_config_parse(text, text ? strlen(text) : 0, c);
}

static void parser_accepts(void)
{
    db_config c;
    assert(parse("version=1\n", &c) && is_default(&c));
    assert(parse("version=1", &c) && is_default(&c)); /* no final newline */
    assert(parse("version=1\nenabled=1\nlayout=extended\n", &c));
    assert(c.enabled == 1 && c.layout == DB_EXTENDED);
    assert(parse("version=1\nlayout=extended\n", &c));
    assert(c.enabled == 0 && c.layout == DB_EXTENDED); /* layout alone: disabled */
    assert(parse("# comment with spaces\n\nversion=1\r\nenabled=1\r\n# x\n", &c));
    assert(c.enabled == 1 && c.layout == DB_COMPACT);
    assert(parse("version=1\nlayout=compact\nenabled=0\n", &c) && is_default(&c));
}

static void parser_rejects(void)
{
    static const char *bad[] = {
        "", "\n", "# only comment\n", "enabled=1\n", "enabled=1\nversion=1\n",
        "version=2\nenabled=1\n", "version=0\n", "version=01\n", "version=1\nversion=1\n",
        "version = 1\n", "version=1 \n", " version=1\n", "\tversion=1\n",
        "version=1\nenabled=1 \n", "version=1\nenabled=2\n", "version=1\nenabled=\n",
        "version=1\nenabled=yes\n", "version=1\nenabled=1\nenabled=1\n",
        "version=1\nenabled=1\nenabled=0\n", "version=1\nlayout=Extended\n",
        "version=1\nlayout=wide\n", "version=1\nlayout=compact\nlayout=compact\n",
        "version=1\nenabled=1\ncolour=red\n", "version=1\nenabled\n",
        "version=1\n=1\n", "version=1\nenabled==1\n", "version=1\n #indented\n",
        "version=1\nenabled=1\nlayout=extended\nx\n", "version=1\nenabled=\xc3\xa9\n",
        "version=1\nenabled=1\x7f\n", "version=1\r\r\n",
    };
    db_config c;
    char buf[DB_CONFIG_TEXT_CAP + 2];
    size_t i;
    for (i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        assert(!parse(bad[i], &c));
        assert(is_default(&c));
    }
    /* NUL byte inside otherwise valid enabling input. */
    memset(&c, 0x5a, sizeof(c));
    assert(!db_config_parse("version=1\nenabled=1\0\n", 21, &c) && is_default(&c));
    /* NULL text, zero length, NULL output. */
    assert(!db_config_parse(NULL, 0, &c) && is_default(&c));
    assert(!db_config_parse(NULL, 5, &c) && is_default(&c));
    assert(!db_config_parse("version=1\n", 10, NULL));
    /* Line length cap: 64 accepted, 65 rejected (comment lines too). */
    memset(buf, 0, sizeof(buf));
    memcpy(buf, "version=1\n#", 11);
    memset(buf + 11, 'x', 63);              /* comment line of exactly 64 bytes */
    assert(db_config_parse(buf, 74, &c));
    buf[74] = 'x';                          /* now 65 bytes */
    assert(!db_config_parse(buf, 75, &c) && is_default(&c));
    /* Whole-input cap: exactly 512 accepted, 513 rejected. */
    memset(buf, '\n', sizeof(buf));
    memcpy(buf, "version=1\nenabled=1\n", 20);
    assert(db_config_parse(buf, DB_CONFIG_TEXT_CAP, &c) && c.enabled == 1);
    assert(!db_config_parse(buf, DB_CONFIG_TEXT_CAP + 1, &c) && is_default(&c));
}

static void validity_and_actions(void)
{
    db_config c, bad;
    db_config_default(&c);
    assert(db_config_valid(&c) && is_default(&c));
    assert(!db_config_valid(NULL));
    bad = c; bad.version = 2; assert(!db_config_valid(&bad));
    bad = c; bad.enabled = 2; assert(!db_config_valid(&bad));
    bad = c; bad.enabled = -1; assert(!db_config_valid(&bad));
    bad = c; bad.layout = (db_layout)7; assert(!db_config_valid(&bad));
    db_config_default(NULL);

    assert(db_config_select_layout(&c, DB_EXTENDED) && !c.enabled && c.layout == DB_EXTENDED);
    assert(db_config_enable(&c) && c.enabled && c.layout == DB_EXTENDED);
    assert(db_config_select_layout(&c, DB_COMPACT) && c.enabled && c.layout == DB_COMPACT);
    assert(db_config_select_layout(&c, DB_EXTENDED) && c.enabled); /* keeps enabled */
    assert(db_config_disable(&c) && !c.enabled && c.layout == DB_EXTENDED);
    db_config_restore_default(&c); assert(is_default(&c));

    c.enabled = 1;
    assert(!db_config_select_layout(&c, (db_layout)9) && is_default(&c));
    bad.enabled = 1; bad.version = 3;
    assert(!db_config_enable(&bad) && is_default(&bad));
    bad.version = 3; assert(!db_config_disable(&bad) && is_default(&bad));
    bad.version = 3; bad.enabled = 1;
    assert(!db_config_select_layout(&bad, DB_EXTENDED) && is_default(&bad));
    assert(!db_config_enable(NULL) && !db_config_disable(NULL));
    assert(!db_config_select_layout(NULL, DB_COMPACT));
    db_config_restore_default(NULL);
}

static void write_round_trip(const char *dir)
{
    char path[512], buf[DB_CONFIG_TEXT_CAP + 1];
    db_config c, back;
    FILE *f;
    size_t n;
    int e, l;
    for (e = 0; e <= 1; ++e)
        for (l = DB_COMPACT; l <= DB_EXTENDED; ++l) {
            c.version = DB_CONFIG_VERSION; c.enabled = e; c.layout = (db_layout)l;
            f = tmpfile(); assert(f);
            assert(db_config_write(f, &c));
            rewind(f);
            n = fread(buf, 1, sizeof(buf), f);
            assert(!fclose(f));
            assert(db_config_parse(buf, n, &back));
            assert(back.enabled == e && back.layout == (db_layout)l);
        }
    c.version = 9;
    f = tmpfile(); assert(f);
    assert(!db_config_write(f, &c) && ftell(f) == 0);
    fclose(f);
    db_config_default(&c);
    assert(!db_config_write(NULL, &c));
    f = fopen("/dev/full", "wb");
    if (f) { setvbuf(f, NULL, _IONBF, 0); assert(!db_config_write(f, &c)); fclose(f); }

    /* Loader: missing, valid, invalid, oversize, directory. */
    snprintf(path, sizeof(path), "%s/missing.cfg", dir);
    remove(path);
    memset(&c, 0x5a, sizeof(c));
    assert(db_config_load_path(path, &c) == DB_CONFIG_MISSING && is_default(&c));
    f = fopen(path, "wb"); assert(f);
    fputs("version=1\nenabled=1\nlayout=extended\n", f); assert(!fclose(f));
    assert(db_config_load_path(path, &c) == DB_CONFIG_LOADED);
    assert(c.enabled == 1 && c.layout == DB_EXTENDED);
    f = fopen(path, "wb"); assert(f);
    fputs("version=1\nenabled=true\n", f); assert(!fclose(f));
    assert(db_config_load_path(path, &c) == DB_CONFIG_INVALID && is_default(&c));
    f = fopen(path, "wb"); assert(f);
    fputs("version=1\nenabled=1\n", f);
    for (n = 0; n < DB_CONFIG_TEXT_CAP; ++n) fputc('\n', f);
    assert(!fclose(f));
    assert(db_config_load_path(path, &c) == DB_CONFIG_INVALID && is_default(&c));
    remove(path);
    assert(db_config_load_path(dir, &c) == DB_CONFIG_INVALID && is_default(&c));
    assert(db_config_load_path(NULL, &c) == DB_CONFIG_INVALID && is_default(&c));
    assert(db_config_load_path(path, NULL) == DB_CONFIG_INVALID);
}

typedef struct { int calls, result; } counter;
static int counting_read(void *context, uint64_t now, db_snapshot *out)
{
    counter *k = context;
    ++k->calls;
    if (k->result != 1) return k->result;
    return db_mock_read(NULL, now, out);
}

static db_config cfg(int enabled, db_layout layout)
{
    db_config c;
    c.version = DB_CONFIG_VERSION; c.enabled = enabled; c.layout = layout;
    return c;
}

static int empty(const db_manager *m)
{
    const db_frame *f = db_manager_frame(m);
    return f->count == 0 && f->width == 0 && !db_frame_valid(f) &&
        m->snapshot.range.sample.availability == DB_MISSING;
}

static int has_text(const db_frame *f, const char *s)
{
    size_t i;
    for (i = 0; i < f->count; ++i)
        if (f->commands[i].kind == DB_DRAW_TEXT && !strcmp(f->commands[i].text, s)) return 1;
    return 0;
}

static void ready(db_manager *m, db_layout expect)
{
    assert(db_manager_update(m, 1000, 800, 252, WIDE));
    assert(db_manager_get_status(m) == DB_MANAGER_READY);
    assert(db_frame_valid(db_manager_frame(m)));
    assert(has_text(db_manager_frame(m), "84.3 km") == (expect == DB_EXTENDED));
    assert(has_text(db_manager_frame(m), "12.5 C"));
}

static void manager(void)
{
    counter k = {0, 1};
    db_provider p = { counting_read, &k };
    db_manager m;
    db_config c;

    /* Default: disabled, provider never read, no drawing. */
    memset(&m, 'X', sizeof(m));
    db_manager_init(&m, &p);
    assert(db_manager_get_status(&m) == DB_MANAGER_OFF_DISABLED && empty(&m));
    assert(is_default(&m.config));
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE) && empty(&m) && k.calls == 0);

    /* Layout selection alone never enables. */
    db_manager_select_layout(&m, DB_EXTENDED);
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE) && empty(&m) && k.calls == 0);
    assert(m.config.layout == DB_EXTENDED && !m.config.enabled);

    /* Enabled config builds; layout switch clears immediately, rebuilds. */
    c = cfg(1, DB_COMPACT); db_manager_set_config(&m, &c);
    assert(empty(&m));
    ready(&m, DB_COMPACT); assert(k.calls == 1);
    db_manager_select_layout(&m, DB_EXTENDED);
    assert(empty(&m) && m.config.enabled);
    ready(&m, DB_EXTENDED);

    /* Provider failure clears, recovery rebuilds. */
    {
        int r;
        for (r = -1; r <= 2; ++r) {
            if (r == 1) continue;
            k.result = 1; ready(&m, DB_EXTENDED);
            k.result = r;
            assert(!db_manager_update(&m, 1000, 800, 252, WIDE) && empty(&m));
            assert(db_manager_get_status(&m) == DB_MANAGER_OFF_PROVIDER);
        }
        k.result = 1; ready(&m, DB_EXTENDED);
    }

    /* Geometry rejection clears (guidance crop, undersized, outside canvas). */
    assert(!db_manager_update(&m, 1000, 210, 153, (cr_rect_t){0, 0, 210, 153}) && empty(&m));
    assert(db_manager_get_status(&m) == DB_MANAGER_OFF_GEOMETRY);
    ready(&m, DB_EXTENDED);
    assert(!db_manager_update(&m, 1000, 700, 252, WIDE) && empty(&m));
    ready(&m, DB_EXTENDED);

    /* Disable clears immediately and stops reads. */
    db_manager_disable(&m);
    assert(empty(&m) && !m.config.enabled && m.config.layout == DB_EXTENDED);
    k.calls = 0;
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE) && k.calls == 0 && empty(&m));

    /* Invalid config: default applied, sticky invalid-config, no reads. */
    c = cfg(1, DB_EXTENDED); db_manager_set_config(&m, &c); ready(&m, DB_EXTENDED);
    c.version = 2; db_manager_set_config(&m, &c);
    assert(empty(&m) && is_default(&m.config));
    k.calls = 0;
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE) && k.calls == 0 && empty(&m));
    assert(db_manager_get_status(&m) == DB_MANAGER_OFF_CONFIG);
    assert(!strcmp(db_manager_status_name(db_manager_get_status(&m)), "invalid-config"));
    db_manager_set_config(&m, NULL);
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE));
    assert(db_manager_get_status(&m) == DB_MANAGER_OFF_CONFIG);
    db_manager_restore_default(&m);
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE));
    assert(db_manager_get_status(&m) == DB_MANAGER_OFF_DISABLED && k.calls == 0);

    /* Restore default from READY: disabled Compact, zero commands. */
    c = cfg(1, DB_EXTENDED); db_manager_set_config(&m, &c); ready(&m, DB_EXTENDED);
    db_manager_restore_default(&m);
    assert(empty(&m) && is_default(&m.config));
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE) && empty(&m));

    /* Corrupted config in state is caught by update. */
    c = cfg(1, DB_COMPACT); db_manager_set_config(&m, &c); ready(&m, DB_COMPACT);
    m.config.layout = (db_layout)42;
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE) && empty(&m));
    assert(db_manager_get_status(&m) == DB_MANAGER_OFF_CONFIG && is_default(&m.config));

    /* Stale data: frame shows placeholders, never old values. */
    c = cfg(1, DB_COMPACT); db_manager_set_config(&m, &c);
    assert(db_manager_update(&m, 0, 800, 252, WIDE)); /* mock stamps now: fresh */
    assert(has_text(db_manager_frame(&m), "12.5 C"));

    /* NULL/missing provider and NULL manager. */
    db_manager_init(&m, NULL);
    c = cfg(1, DB_COMPACT); db_manager_set_config(&m, &c);
    assert(!db_manager_update(&m, 1000, 800, 252, WIDE) && empty(&m));
    assert(db_manager_get_status(&m) == DB_MANAGER_OFF_PROVIDER);
    db_manager_init(NULL, &p); db_manager_set_config(NULL, &c);
    db_manager_disable(NULL); db_manager_restore_default(NULL);
    db_manager_select_layout(NULL, DB_COMPACT);
    assert(!db_manager_update(NULL, 0, 800, 252, WIDE));
    assert(db_manager_frame(NULL) == NULL);
    assert(db_manager_get_status(NULL) == DB_MANAGER_OFF_CONFIG);
    assert(!strcmp(db_manager_status_name(DB_MANAGER_READY), "ready"));
    assert(!strcmp(db_manager_status_name(DB_MANAGER_OFF_DISABLED), "disabled"));
    assert(!strcmp(db_manager_status_name(DB_MANAGER_OFF_PROVIDER), "provider-failure"));
    assert(!strcmp(db_manager_status_name(DB_MANAGER_OFF_GEOMETRY), "unsupported-geometry"));
    assert(!strcmp(db_manager_status_name((db_manager_status)99), "unknown"));
}

int main(int argc, char **argv)
{
    assert(argc == 2); /* scratch directory for loader fixtures */
    parser_accepts(); parser_rejects(); validity_and_actions();
    write_round_trip(argv[1]); manager();
    puts("dashboard_manager_test: config parse/limits/actions/round trip/loader, manager gating, clearing, provider/geometry/config fallback PASS");
    return 0;
}
