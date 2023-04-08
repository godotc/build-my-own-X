#include "constant.h"
#include "line.h"
#include "triangle.h"


#include "geometry.h"
#include "model.h"
#include "tgaimage.h"
#include <cmath>
#include <memory>
#include <sys/types.h>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>



void rectangle(int x1, int y1, int x2, int y2, TGAImage &image, const TGAColor &color)
{
    for (int i = x1; i < x2; ++i) {
        for (int j = x2; j < y2; ++j) {
            image.set(i, j, color);
        }
    }
}

void drawObj(TGAImage &image, const char *source)
{
    auto model = std::make_shared<Model>(source);
    // model = std::make_shared<Model>("./obj/african_head.obj");

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
}



int main(int argc, char **argv)
{
    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

    if (2 == argc) {
        drawObj(image, argv[1]);
    }

    Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    triangle_v1(t0[0], t0[1], t0[2], image, red);
    triangle_v1(t1[0], t1[1], t1[2], image, white);
    triangle_v0(t2[0], t2[1], t2[2], image, green);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}
