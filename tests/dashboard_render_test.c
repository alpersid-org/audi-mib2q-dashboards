/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "../dashboard/dashboard.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static db_snapshot snapshot;

static int has_text(const db_frame *f, const char *s)
{
    size_t i;
    for (i = 0; i < f->count; ++i)
        if (f->commands[i].kind == DB_DRAW_TEXT && !strcmp(f->commands[i].text, s)) return 1;
    return 0;
}

static void layouts(void)
{
    const int sizes[][2] = { {800,252}, {640,360}, {640,240}, {1600,900} };
    db_frame f;
    size_t i, j;
    int mode;
    for (mode = DB_COMPACT; mode <= DB_EXTENDED; ++mode) {
        for (i = 0; i < sizeof(sizes)/sizeof(sizes[0]); ++i) {
            int w = sizes[i][0], h = sizes[i][1], badges = 0;
            cr_rect_t clip = {30, 20, (float)w, (float)h};
            assert(db_layout_build((db_layout)mode, &snapshot, 1000, w+60, h+40, clip, &f));
            assert(db_frame_valid(&f) && f.count <= DB_COMMAND_CAP);
            assert(cr_rect_equal(f.clip, clip));
            assert(has_text(&f, "HOST PREVIEW - NO VEHICLE OUTPUT"));
            assert(has_text(&f, "12.5 C") && has_text(&f, "620 km") && has_text(&f, "Night Drive"));
            /* Compact: combined navigation card. Extended (P2-07): split cards from the
             * same navigation sample; no trip distance or consumption anywhere. */
            assert(has_text(&f, "42.5 km / 38 min") == (mode == DB_COMPACT));
            assert(has_text(&f, "42.5 km") == (mode == DB_EXTENDED));
            assert(has_text(&f, "38 min") == (mode == DB_EXTENDED));
            assert(has_text(&f, "City Centre"));
            assert(has_text(&f, "DISTANCE TO DESTINATION") == (mode == DB_EXTENDED));
            assert(has_text(&f, "TIME TO DESTINATION") == (mode == DB_EXTENDED));
            assert(!has_text(&f, "5.6 L/100km") && !has_text(&f, "84.3 km"));
            assert(!has_text(&f, "TRIP DISTANCE") && !has_text(&f, "AVERAGE CONSUMPTION"));
            for (j = 0; j < f.count; ++j) {
                const db_draw_command *c = &f.commands[j];
                assert(c->box.x >= clip.x && c->box.y >= clip.y);
                assert(c->box.x+c->box.w <= clip.x+clip.w);
                assert(c->box.y+c->box.h <= clip.y+clip.h);
                if (c->kind == DB_DRAW_TEXT && !strcmp(c->text, "MOCK")) ++badges;
            }
            assert(badges == (mode == DB_COMPACT ? 4 : 6));
        }
    }
}

static void failure_clears(void)
{
    const cr_rect_t bad[] = {
        {0,0,210,153}, {0,0,328,180}, {0,0,639,252}, {0,0,800,239},
        {-1,0,800,252}, {0,0,801,252}, {0,0,800,253}, {1,0,800,252},
        {0,0,0,0}, {NAN,0,800,252}, {0,INFINITY,800,252},
        {0,0,INFINITY,252}, {0,0,800,NAN}, {.5f,0,640,240}, {0,0,640.5f,240}
    };
    db_frame f;
    size_t i;
    for (i = 0; i < sizeof(bad)/sizeof(bad[0]); ++i) {
        assert(db_layout_build(DB_COMPACT, &snapshot, 1000, 800,252,(cr_rect_t){0,0,800,252}, &f));
        assert(!db_layout_build(DB_COMPACT, &snapshot,1000,800,252,bad[i],&f));
        assert(f.count == 0 && f.width == 0 && !db_frame_valid(&f));
    }
    memset(&f, 'X', sizeof(f));
    assert(!db_layout_build((db_layout)99,&snapshot,1000,800,252,(cr_rect_t){0,0,800,252},&f));
    assert(f.count == 0);
    assert(!db_layout_build(DB_COMPACT,NULL,1000,800,252,(cr_rect_t){0,0,800,252},&f));
    assert(!db_layout_build(DB_COMPACT,&snapshot,1000,INT_MAX,252,(cr_rect_t){0,0,800,252},&f));
    assert(!db_layout_build(DB_COMPACT,&snapshot,1000,800,0,(cr_rect_t){0,0,800,252},&f));
    assert(!db_layout_build(DB_COMPACT,&snapshot,1000,800,252,(cr_rect_t){0,0,800,252},NULL));
}

static void placeholders_and_elision(void)
{
    db_snapshot s = snapshot;
    db_frame f;
    size_t i;
    int elided = 0;
    s.temperature.sample.availability = DB_MISSING;
    s.range.sample.sampled_ms = 0;
    memset(s.media.title, 'W', sizeof(s.media.title)-1);
    s.media.title[sizeof(s.media.title)-1] = 0;
    assert(db_layout_build(DB_COMPACT,&s,DB_MAX_AGE_MS+1,640,360,(cr_rect_t){0,0,640,360},&f));
    assert(has_text(&f, "UNAVAILABLE") && has_text(&f, "STALE") && has_text(&f,"--"));
    assert(!has_text(&f, "12.5 C") && !has_text(&f,"620 km"));
    for (i = 0; i < f.count; ++i) {
        db_draw_command *c = &f.commands[i];
        if (c->kind == DB_DRAW_TEXT && strstr(c->text,"...")) ++elided;
    }
    assert(elided > 0 && db_frame_valid(&f));
}

static void invalid_svg_writes_nothing(db_frame *f)
{
    FILE *stream = tmpfile();
    assert(stream && !db_frame_valid(f));
    assert(!db_frame_write_svg(stream,f));
    assert(ftell(stream) == 0);
    assert(!fclose(stream));
}

static void exporter(const char *fixture)
{
    db_frame f, bad;
    FILE *stream;
    size_t text_index = 0;
    assert(db_layout_build(DB_EXTENDED,&snapshot,1000,800,252,(cr_rect_t){0,0,800,252},&f));
    while (f.commands[text_index].kind != DB_DRAW_TEXT) ++text_index;
    bad=f; bad.count=DB_COMMAND_CAP+1; invalid_svg_writes_nothing(&bad);
    bad=f; bad.commands[0].kind=(db_draw_kind)99; invalid_svg_writes_nothing(&bad);
    bad=f; bad.commands[0].rgb=0x1000000; invalid_svg_writes_nothing(&bad);
    bad=f; bad.commands[0].box.x=-1; invalid_svg_writes_nothing(&bad);
    bad=f; bad.commands[0].box.w=NAN; invalid_svg_writes_nothing(&bad);
    bad=f; bad.commands[text_index].font_size=INT_MAX; invalid_svg_writes_nothing(&bad);
    bad=f; bad.commands[text_index].box.h=1; invalid_svg_writes_nothing(&bad);
    bad=f; memset(bad.commands[text_index].text,'A',DB_TEXT_CAP); invalid_svg_writes_nothing(&bad);
    bad=f; bad.commands[text_index].text[0]='\n'; invalid_svg_writes_nothing(&bad);
    assert(!db_frame_write_svg(NULL,&f) && !db_frame_valid(NULL));
    stream=tmpfile(); assert(stream && !db_frame_write_svg(stream,NULL)); fclose(stream);
    strcpy(f.commands[text_index].text, "<script>&\"' >");
    stream=fopen(fixture,"wb"); assert(stream);
    assert(db_frame_write_svg(stream,&f)); assert(!fclose(stream));
    /* Linux optional sink verifies observed I/O errors propagate. */
    stream=fopen("/dev/full","wb");
    if (stream) { setvbuf(stream,NULL,_IONBF,0); assert(!db_frame_write_svg(stream,&f)); fclose(stream); }
}

int main(int argc, char **argv)
{
    assert(argc==2 && db_mock_read(NULL,1000,&snapshot));
    layouts(); failure_clears(); placeholders_and_elision(); exporter(argv[1]);
    puts("dashboard_render_test: layout content/bounds, crops/clearing, placeholders/elision, invalid frames/XML and I/O errors PASS");
    return 0;
}
