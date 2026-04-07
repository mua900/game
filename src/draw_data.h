#ifndef _DRAW_DATA_H
#define _DRAW_DATA_H

struct DrawData {
    ColorF color;

    DrawData() {}
    DrawData(ColorF color) : color(color) {}
};

#define LINE_DRAW_DATA_MAX_POINTS 8
struct LineDrawData {
    int point_count = 0;
    vec2 points[LINE_DRAW_DATA_MAX_POINTS];
};

#endif // _DRAW_DATA_H