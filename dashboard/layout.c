/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <string.h>

static int rect(db_frame *f, int x, int y, int w, int h, uint32_t rgb)
{
    db_draw_command *c;
    if (f->count >= DB_COMMAND_CAP) return 0;
    c = &f->commands[f->count++];
    c->kind = DB_DRAW_RECT;
    c->box = (cr_rect_t){(float)x, (float)y, (float)w, (float)h};
    c->rgb = rgb;
    return 1;
}

/* DejaVu Sans Mono advance is < .65em. Reserve one pixel per character beyond
 * floor(.65*size), elide with dots, and still clip in SVG. No font measurement
 * dependency; never shrink units/numbers to unreadable sizes to fit. */
static int text(db_frame *f, int x, int y, int w, int size, uint32_t rgb, const char *s)
{
    db_draw_command *c;
    size_t n = strlen(s), max = (size_t)(w / ((size * 65 + 99) / 100));
    if (f->count >= DB_COMMAND_CAP || max < 3) return 0;
    if (max >= DB_TEXT_CAP) max = DB_TEXT_CAP - 1;
    c = &f->commands[f->count++];
    c->kind = DB_DRAW_TEXT;
    c->box = (cr_rect_t){(float)x, (float)y, (float)w, (float)(size + 6)};
    c->rgb = rgb; c->font_size = size;
    if (n <= max) memcpy(c->text, s, n + 1);
    else { memcpy(c->text, s, max - 3); memcpy(c->text + max - 3, "...", 4); }
    return 1;
}

static int card(db_frame *f, db_widget widget, const db_snapshot *s, uint64_t now,
                int x, int y, int w, int h)
{
    db_widget_view v;
    int content = w - 24, size, badge_w;
    uint32_t badge_color;
    db_widget_format(widget, s, now, &v);
    size = widget == DB_NAVIGATION ? (content < 200 ? 14 : 16) :
           widget == DB_MEDIA || widget == DB_DESTINATION ? 20 : 24;
    badge_w = (int)strlen(v.badge) * 6;
    badge_color = v.status != DB_VALUE_OK ? 0xf0bd70 :
                  v.source == DB_SOURCE_MOCK ? 0x78bde8 : 0xc9d0d8;
    return rect(f, x, y, w, h, 0x141a21) &&
        rect(f, x, y, 2, h, 0x424c59) &&
        text(f, x + 12, y + 7, content, 10, 0xc9d0d8, v.label) &&
        text(f, x + 12, y + h/2 - 12, content, size, 0xf5f7fa, v.value) &&
        text(f, x + 12, y + h - 20, content - badge_w - 8, 10, 0xaab5c2, v.detail) &&
        text(f, x + w - 12 - badge_w, y + h - 19, badge_w, 9, badge_color, v.badge);
}

int db_layout_build(db_layout layout, const db_snapshot *snapshot, uint64_t now_ms,
                    int width, int height, cr_rect_t area, db_frame *out)
{
    static const db_widget compact[] = { DB_TEMPERATURE, DB_RANGE, DB_MEDIA, DB_NAVIGATION };
    static const db_widget extended[] = {
        /* P2-07: same verified signal groups as Compact, in more detail. */
        DB_TEMPERATURE, DB_RANGE, DB_MEDIA, DB_NAV_DISTANCE, DB_NAV_TIME, DB_DESTINATION
    };
    const db_widget *widgets = layout == DB_COMPACT ? compact : extended;
    int cols = layout == DB_COMPACT ? 2 : 3;
    int x, y, w, h, i, total, card_w, card_h, top, grid_h, ok;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->width = width; out->height = height; out->clip = area;
    /* Validate before float-to-int conversions, including NaN/infinity/fractions. */
    if (!snapshot || (layout != DB_COMPACT && layout != DB_EXTENDED) ||
        !db_frame_valid(out) || area.w < 640 || area.h < 240) goto fail;
    x = (int)area.x; y = (int)area.y; w = (int)area.w; h = (int)area.h;
    top = y + 56; grid_h = h - 78;
    card_w = (w - 32 - (cols - 1) * 10) / cols;
    card_h = (grid_h - 10) / 2;
    ok = rect(out, x, y, w, h, 0x080b10) &&
        rect(out, x + 16, y + 10, 3, 24, 0xdc3545) &&
        text(out, x + 28, y + 9, w - 44, 18, 0xf5f7fa, "CUSTOM DASHBOARDS") &&
        text(out, x + 16, y + 35, w - 32, 10, 0xaab5c2,
             layout == DB_COMPACT ? "COMPACT / HOST PREVIEW" : "EXTENDED / HOST PREVIEW") &&
        rect(out, x + 16, y + h - 19, w - 32, 1, 0x303945) &&
        text(out, x + 16, y + h - 16, w - 32, 9, 0xaab5c2,
             "HOST PREVIEW - NO VEHICLE OUTPUT");
    total = cols * 2;
    for (i = 0; ok && i < total; ++i)
        ok = card(out, widgets[i], snapshot, now_ms,
                  x + 16 + (i % cols) * (card_w + 10),
                  top + (i / cols) * (card_h + 10), card_w, card_h);
    if (!ok || !db_frame_valid(out)) goto fail;
    return 1;
fail:
    memset(out, 0, sizeof(*out));
    return 0;
}
