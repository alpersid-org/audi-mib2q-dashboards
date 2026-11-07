/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dashboard.h"
#include <math.h>
#include <string.h>

static int box_valid(cr_rect_t b)
{
    /* Bounds precede casts so extreme floats, infinity and NaN cannot reach int. */
    return isfinite(b.x) && isfinite(b.y) && isfinite(b.w) && isfinite(b.h) &&
        b.x >= 0 && b.y >= 0 && b.w > 0 && b.h > 0 &&
        b.x <= 4096 && b.y <= 2160 && b.w <= 4096 && b.h <= 2160 &&
        b.x == (int)b.x && b.y == (int)b.y && b.w == (int)b.w && b.h == (int)b.h;
}

static int inside(cr_rect_t b, cr_rect_t a)
{
    return b.x >= a.x && b.y >= a.y && b.x + b.w <= a.x + a.w && b.y + b.h <= a.y + a.h;
}

int db_frame_valid(const db_frame *f)
{
    size_t i, j;
    cr_rect_t canvas;
    if (!f || f->width <= 0 || f->height <= 0 || f->width > 4096 || f->height > 2160 ||
        f->count > DB_COMMAND_CAP || !box_valid(f->clip)) return 0;
    canvas = (cr_rect_t){0, 0, (float)f->width, (float)f->height};
    if (!inside(f->clip, canvas)) return 0;
    for (i = 0; i < f->count; ++i) {
        const db_draw_command *c = &f->commands[i];
        if (!box_valid(c->box) || !inside(c->box, f->clip) || c->rgb > 0xffffff) return 0;
        if (c->kind == DB_DRAW_RECT) continue;
        if (c->kind != DB_DRAW_TEXT || c->font_size < 8 || c->font_size > 64 ||
            c->box.h < c->font_size + 6 || !memchr(c->text, 0, DB_TEXT_CAP)) return 0;
        for (j = 0; c->text[j]; ++j)
            if ((unsigned char)c->text[j] < 32 || (unsigned char)c->text[j] > 126) return 0;
    }
    return 1;
}

static void svg_rect(FILE *s, const char *tag, cr_rect_t b)
{
    fprintf(s, "<%s x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"/>",
            tag, (int)b.x, (int)b.y, (int)b.w, (int)b.h);
}

static void escaped(FILE *s, const char *text)
{
    for (; *text; ++text) {
        switch (*text) {
        case '&': fputs("&amp;", s); break;
        case '<': fputs("&lt;", s); break;
        case '>': fputs("&gt;", s); break;
        case '"': fputs("&quot;", s); break;
        case '\'': fputs("&apos;", s); break;
        default: fputc(*text, s); break;
        }
    }
}

int db_frame_write_svg(FILE *s, const db_frame *f)
{
    size_t i;
    if (!s || !db_frame_valid(f)) return 0;
    fprintf(s, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\">\n",
            f->width, f->height, f->width, f->height);
    fputs("<title>Custom dashboards - host preview, no vehicle output</title>\n<defs>\n<clipPath id=\"viewport\">", s);
    svg_rect(s, "rect", f->clip);
    fputs("</clipPath>\n", s);
    for (i = 0; i < f->count; ++i) if (f->commands[i].kind == DB_DRAW_TEXT) {
        fprintf(s, "<clipPath id=\"text%zu\">", i);
        svg_rect(s, "rect", f->commands[i].box);
        fputs("</clipPath>\n", s);
    }
    fputs("</defs>\n<g clip-path=\"url(#viewport)\" font-family=\"DejaVu Sans Mono,monospace\">\n", s);
    for (i = 0; i < f->count; ++i) {
        const db_draw_command *c = &f->commands[i];
        if (c->kind == DB_DRAW_RECT) {
            fprintf(s, "<g fill=\"#%06lx\">", (unsigned long)c->rgb);
            svg_rect(s, "rect", c->box);
            fputs("</g>\n", s);
        } else {
            fprintf(s, "<text x=\"%d\" y=\"%d\" font-size=\"%d\" fill=\"#%06lx\" clip-path=\"url(#text%zu)\">",
                    (int)c->box.x, (int)c->box.y + c->font_size, c->font_size, (unsigned long)c->rgb, i);
            escaped(s, c->text); fputs("</text>\n", s);
        }
    }
    fputs("</g>\n</svg>\n", s);
    return !ferror(s);
}
