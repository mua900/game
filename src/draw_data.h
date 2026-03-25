#ifndef _DRAW_DATA_H
#define _DRAW_DATA_H

struct DrawData {
    ColorF color;

    DrawData() {}
    DrawData(ColorF color) : color(color) {}
};

struct LineDrawData {
    DArray<vec2> points;
};

#endif // _DRAW_DATA_H