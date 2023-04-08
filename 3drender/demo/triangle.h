#pragma once

#include "constant.h"
#include "geometry.h"
#include "line.h"
#include "tgaimage.h"
#include <array>
#include <vector>

using point = Vec2i;


/**
 * @brief Original draw a triangle from 3 point.
 (Must read!!)
 Setps:
   1. Sort 3 point by y (height), t0 < t1 < t2
   2. Seprate triangle to 2 segment: upper triangle and bottom triangle
   3. From lowest/heighest point, calulate left and right point on 2 edges
   4. Draw horizental lines betwen 2 points (fix pixels)
 Pseudo-code:
 ```cpp
 triangle(vec2 points[3]) {
    vec2 bbox[2] = find_bounding_box(points);
    for (each pixel in the bounding box) {
        if (inside(points, pixel)) {
            put_pixel(pixel);
        }
    }
 }
 ```
 */
void triangle_v0(point p0, point p1, point p2, TGAImage &image, const TGAColor &color)
{
    // height of y: t0<t1<t2
    if (p0.y > p1.y) std::swap(p0, p1);
    if (p0.y > p2.y) std::swap(p0, p2);
    if (p1.y > p2.y) std::swap(p1, p2);
    line(p0, p1, image, color);
    line(p1, p2, image, color);
    line(p2, p0, image, color);

    int total_height = p2.y - p0.y;

    int bottom_segment_height = p1.y - p0.y + 1; // avoid divide by 0
    int upper_segment_height  = p2.y - p1.y + 1; // avoid divide by 0

    // lower
    for (int y = p0.y; y < p1.y; ++y) {

        float current_height = y - p0.y;

        // get the ratio of point's y  on 2 edges
        float alpha = (float)current_height / total_height;          // cur height : long edge
        float beta  = (float)current_height / bottom_segment_height; // cur height : short edge

        Vec2i A = p0 + (p2 - p0) * alpha; // add t(02)->  get point on this edge (two sides)
        Vec2i B = p0 + (p1 - p0) * beta;  // add t(01)->

        if (A.x > B.x) std::swap(A, B);
        for (int x = A.x; x <= B.x; ++x) {
            image.set(x, y, red);
        }
    }

    // upper
    for (int y = p2.y; y >= p1.y; --y) {
        // float current_height = y - p1.y; // wrong: waht get is the left distace
        float current_height = p2.y - y;

        float alpha = (float)current_height / total_height;
        float beta  = (float)current_height / upper_segment_height;

        Vec2i A = p2 + (p0 - p2) * alpha;
        Vec2i B = p2 + (p1 - p2) * beta;

        if (A.x > B.x) std::swap(A, B);
        for (int x = A.x; x <= B.x; ++x) {
            image.set(x, y, green);
        }
        // line({A.x, y}, {B.x, y}, image, color);
    }
}

void triangle_v1(point p0, point p1, point p2, TGAImage &image, const TGAColor &color)
{
    if (p0.y & p1.y || p1.y & p2.y) return; // Did not care about `degenreated triangle`
    if (p0.y > p1.y) std::swap(p0, p1);
    if (p0.y > p2.y) std::swap(p0, p2);
    if (p1.y > p2.y) std::swap(p1, p2);

    line(p0, p1, image, color);
    line(p1, p2, image, color);
    line(p2, p0, image, color);

    int total_height = p2.y - p0.y;
    for (int i = 0; i < total_height; ++i) {
        bool is_upper_segment = i > (p1.y - p0.y) || p1.y == p2.y; // is second half segment
        int  segment_height   = is_upper_segment ? p2.y - p1.y : p1.y - p0.y;

        float alpha = (float)i / total_height;
        float beta  = (float)(i - (is_upper_segment ? p1.y - p0.y : 0)) / segment_height; // remove lower height section when on upper triangle

        Vec2i A = p0 + (p2 - p0) * alpha;
        // Vec2i B = (is_upper_segment ? p1 + (p2 - p1)*beta : p0 + (p1 - p0)) * beta; // bug: multipaly the origin point by beta
        Vec2i B = is_upper_segment ? p1 + (p2 - p1) * beta : p0 + (p1 - p0) * beta;

        if (A.x > B.x) std::swap(A, B);
        for (int j = A.x; j <= B.x; ++j) {
            image.set(j, p0.y + i, color);
        }
    }
}


Vec3f barycentric(std::array<Vec2i, 3> &&points, Vec2i P)
{
}