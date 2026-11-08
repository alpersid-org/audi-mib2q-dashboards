/* SPDX-License-Identifier: GPL-3.0-or-later */
/* P2-03 strict versioned host configuration. Fails closed to disabled Compact. */
#define _POSIX_C_SOURCE 200809L
#include "dashboard.h"
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

void db_config_default(db_config *out)
{
    if (!out) return;
    out->version = DB_CONFIG_VERSION;
    out->enabled = 0;
    out->layout = DB_COMPACT;
}

int db_config_valid(const db_config *c)
{
    return c && c->version == DB_CONFIG_VERSION &&
        (c->enabled == 0 || c->enabled == 1) &&
        (c->layout == DB_COMPACT || c->layout == DB_EXTENDED);
}

static int eq(const char *s, size_t n, const char *lit)
{
    return strlen(lit) == n && memcmp(s, lit, n) == 0;
}

enum { SEEN_VERSION = 1, SEEN_ENABLED = 2, SEEN_LAYOUT = 4 };

/* One "key=value" line without newline (a trailing CR was already removed). */
static int parse_line(const char *s, size_t n, db_config *c, unsigned *seen)
{
    const char *sep = memchr(s, '=', n);
    const char *v;
    size_t i, k, vn;
    if (n == 0 || s[0] == '#') return 1; /* blank or full-line comment */
    for (i = 0; i < n; ++i) /* printable ASCII only, no spaces/tabs anywhere */
        if ((unsigned char)s[i] <= 0x20 || (unsigned char)s[i] >= 0x7f) return 0;
    if (!sep) return 0;
    k = (size_t)(sep - s); v = sep + 1; vn = n - k - 1;
    if (!(*seen & SEEN_VERSION)) { /* version must be the first key */
        if (!eq(s, k, "version") || !eq(v, vn, "1")) return 0;
        *seen |= SEEN_VERSION;
        return 1;
    }
    if (eq(s, k, "enabled")) {
        if (*seen & SEEN_ENABLED) return 0;
        *seen |= SEEN_ENABLED;
        if (eq(v, vn, "0")) c->enabled = 0;
        else if (eq(v, vn, "1")) c->enabled = 1;
        else return 0;
        return 1;
    }
    if (eq(s, k, "layout")) {
        if (*seen & SEEN_LAYOUT) return 0;
        *seen |= SEEN_LAYOUT;
        if (eq(v, vn, "compact")) c->layout = DB_COMPACT;
        else if (eq(v, vn, "extended")) c->layout = DB_EXTENDED;
        else return 0;
        return 1;
    }
    return 0; /* unknown or repeated version key */
}

int db_config_parse(const char *text, size_t len, db_config *out)
{
    db_config c;
    unsigned seen = 0;
    size_t pos = 0;
    db_config_default(&c);
    if (!out) return 0;
    if (!text || len == 0 || len > DB_CONFIG_TEXT_CAP ||
        memchr(text, '\0', len)) goto fail;
    while (pos < len) {
        const char *line = text + pos;
        const char *nl = memchr(line, '\n', len - pos);
        size_t n = nl ? (size_t)(nl - line) : len - pos;
        pos += n + (nl ? 1 : 0);
        if (n && line[n - 1] == '\r') --n; /* tolerate CRLF files */
        if (n > DB_CONFIG_LINE_CAP || !parse_line(line, n, &c, &seen)) goto fail;
    }
    if (!(seen & SEEN_VERSION) || !db_config_valid(&c)) goto fail;
    *out = c;
    return 1;
fail:
    db_config_default(out);
    return 0;
}

db_config_load db_config_load_path(const char *path, db_config *out)
{
    char buf[DB_CONFIG_TEXT_CAP + 1];
    ssize_t n;
    int fd, flags = O_RDONLY | O_NONBLOCK;
    struct stat before, opened;
    db_config_default(out);
    if (!out || !path) return DB_CONFIG_INVALID;
    errno = 0;
    if (lstat(path, &before) != 0)
        return errno == ENOENT ? DB_CONFIG_MISSING : DB_CONFIG_INVALID;
    if (!S_ISREG(before.st_mode)) return DB_CONFIG_INVALID;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    fd = open(path, flags);
    if (fd < 0) return DB_CONFIG_INVALID;
    if (fstat(fd, &opened) != 0 || !S_ISREG(opened.st_mode) ||
#ifndef O_NOFOLLOW
        opened.st_dev != before.st_dev || opened.st_ino != before.st_ino ||
#endif
        opened.st_size > DB_CONFIG_TEXT_CAP) { close(fd); return DB_CONFIG_INVALID; }
    n = read(fd, buf, sizeof(buf));
    if (close(fd) != 0 || n < 0 || n > DB_CONFIG_TEXT_CAP) return DB_CONFIG_INVALID;
    return db_config_parse(buf, (size_t)n, out) ? DB_CONFIG_LOADED : DB_CONFIG_INVALID;
}

int db_config_write(FILE *stream, const db_config *c)
{
    if (!stream || !db_config_valid(c)) return 0;
    if (fprintf(stream, "# Custom dashboards host configuration (no vehicle output)\n"
                "version=%d\nenabled=%d\nlayout=%s\n", c->version, c->enabled,
                c->layout == DB_COMPACT ? "compact" : "extended") < 0) return 0;
    return !ferror(stream);
}

int db_config_enable(db_config *c)
{
    if (!db_config_valid(c)) { db_config_default(c); return 0; }
    c->enabled = 1;
    return 1;
}

int db_config_disable(db_config *c)
{
    if (!db_config_valid(c)) { db_config_default(c); return 0; }
    c->enabled = 0;
    return 1;
}

int db_config_select_layout(db_config *c, db_layout layout)
{
    if (!db_config_valid(c) || (layout != DB_COMPACT && layout != DB_EXTENDED)) {
        db_config_default(c);
        return 0;
    }
    c->layout = layout; /* enablement intentionally unchanged */
    return 1;
}

void db_config_restore_default(db_config *c)
{
    db_config_default(c);
}

int db_config_store_path(const char *path, const db_config *c)
{
    char tmp[4096];
    FILE *f;
    int fd, ok;
    if (!path || !db_config_valid(c)) return 0;
    if ((size_t)snprintf(tmp, sizeof(tmp), "%s.tmp.%ld", path, (long)getpid()) >= sizeof(tmp))
        return 0;
    fd = open(tmp, O_WRONLY | O_CREAT | O_EXCL, 0644);   /* never follow/clobber */
    if (fd < 0) return 0;
    f = fdopen(fd, "w");
    if (!f) { close(fd); unlink(tmp); return 0; }
    ok = db_config_write(f, c) && fflush(f) == 0 && fsync(fileno(f)) == 0;
    if (fclose(f) != 0) ok = 0;
    if (ok && rename(tmp, path) != 0) ok = 0;
    if (!ok) unlink(tmp);
    return ok;
}
