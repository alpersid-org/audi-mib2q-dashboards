/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-C05 settings contract: runs the shared conformance vectors
 * (tests/fixtures/dashboard_config_vectors.txt) through config.c. The Java side
 * (DashboardConfigTest) runs the same file through DashboardConfig.parse. */
#include "../dashboard/dashboard.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int hexval(int c)
{
    return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
}

int main(int argc, char **argv)
{
    static char line[4096];
    static unsigned char data[2048];
    char name[128], expect[128];
    FILE *f;
    int cases = 0, have_case = 0;
    assert(argc == 2);
    f = fopen(argv[1], "r");
    assert(f);
    while (fgets(line, sizeof(line), f)) {
        size_t n, i, len = 0;
        db_config c;
        int ok;
        n = strlen(line);
        assert(n && line[n - 1] == '\n');           /* no truncated vector lines */
        line[--n] = 0;
        if (!n || line[0] == '#') continue;
        if (!strncmp(line, "case ", 5)) {
            char *sp = strchr(line + 5, ' ');
            assert(sp && !have_case && (size_t)(sp - line - 5) < sizeof(name));
            memcpy(name, line + 5, (size_t)(sp - line - 5)); name[sp - line - 5] = 0;
            assert(strlen(sp + 1) < sizeof(expect));
            strcpy(expect, sp + 1);
            have_case = 1;
            continue;
        }
        assert(have_case && !strncmp(line, "hex ", 4));
        if (strcmp(line + 4, "-")) {
            assert((n - 4) % 2 == 0 && (n - 4) / 2 <= sizeof(data));
            for (i = 4; i < n; i += 2) {
                int hi = hexval(line[i]), lo = hexval(line[i + 1]);
                assert(hi >= 0 && lo >= 0);
                data[len++] = (unsigned char)(hi << 4 | lo);
            }
        }
        memset(&c, 0x5a, sizeof(c));
        ok = db_config_parse(len ? (const char *)data : NULL, len, &c);
        if (!strcmp(expect, "invalid")) {
            if (ok) { fprintf(stderr, "vector %s: accepted, expected invalid\n", name); return 1; }
            assert(!c.enabled && c.layout == DB_COMPACT && c.version == DB_CONFIG_VERSION);
        } else {
            int en; char lay[32];
            assert(sscanf(expect, "valid enabled=%d layout=%31s", &en, lay) == 2);
            if (!ok || c.enabled != en ||
                c.layout != (!strcmp(lay, "extended") ? DB_EXTENDED : DB_COMPACT)) {
                fprintf(stderr, "vector %s: got ok=%d enabled=%d layout=%d, expected %s\n",
                        name, ok, c.enabled, (int)c.layout, expect);
                return 1;
            }
        }
        have_case = 0;
        ++cases;
    }
    assert(!have_case && !fclose(f) && cases >= 40);
    printf("dashboard_config_vectors_test: %d shared v1 config vectors match config.c PASS\n", cases);
    return 0;
}
