#pragma once

#include "geometry.h"
#include "tgaimage.h"


void line(int x1, int y1, int x2, int y2, TGAImage &image, const TGAColor &color)
{
    bool steep = false;

    // x shorten than y
    if (std::abs(x1 - x2) < std::abs(y1 - y2)) {
        std::swap(x1, y1);
        std::swap(x2, y2);
        steep = true;
    }

    // make it left to right
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }


    for (int x = x1; x <= x2; ++x) {
        // one dimenson as long other step draw as ratio
        float t = (x - x1) / (float)(x2 - x1);
        float y = y1 * (1 - t) + y2 * t;

        if (steep) image.set(y, x, color);
        else image.set(x, y, color);
    }
}

using point = Vec2i;
void line(point &&p1, point &&p2, TGAImage &image, const TGAColor &color)
{
    line(p1.x, p1.y, p2.x, p2.y, image, color);
}
void line(point &p1, point &p2, TGAImage &image, const TGAColor &color)
{
    line(p1.x, p1.y, p2.x, p2.y, image, color);
}