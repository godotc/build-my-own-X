#include "geometry.h"
#include "model.h"
#include "tgaimage.h"
#include <cmath>
#include <memory>
#include <sys/types.h>
#include <system_error>
#include <vector>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);


void rectangle(int x1, int y1, int x2, int y2, TGAImage &image, const TGAColor &color)
{
    for (int i = x1; i < x2; ++i) {
        for (int j = x2; j < y2; ++j) {
            image.set(i, j, color);
        }
    }
}

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

        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}
using point = Vec2i;
void line(point &p1, point &p2, TGAImage &image, const TGAColor &color)
{
    line(p1.x, p1.y, p2.x, p2.y, image, color);
}


void triangle(point t0, point t1, point t2, TGAImage &image, const TGAColor &color)
{
    line(t0, t1, image, color);
    line(t1, t2, image, color);
    line(t2, t0, image, color);
}

const int WIDTH  = 800;
const int HEIGHT = 800;

int main(int argc, char **argv)
{
    std::shared_ptr<Model> model;

    if (2 == argc) {
        model = std::make_shared<Model>(argv[1]);
    }
    else {
        model = std::make_shared<Model>("./obj/african_head.obj");
    }
    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

    for (int i = 0; i < model->nfaces(); ++i) {
        std::vector<int> face = model->face(i);

        // Triangle
        for (int j = 0; j < 3; ++j) {
            Vec3f &v0 = model->vert(face[j]);
            Vec3f  v1 = model->vert(face[(j + 1) % 3]);

            // -0.5 or 0.5 to add 1, then multiply by screen height/widh
            int x0 = (v0.x + 1.f) * WIDTH / 2.f;
            int y0 = (v0.y + 1.f) * HEIGHT / 2.f;
            int x1 = (v1.x + 1.f) * WIDTH / 2.f;
            int y1 = (v1.y + 1.f) * HEIGHT / 2.f;

            line(x0, y0, x1, y1, image, white);
        }
    }
    Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}
