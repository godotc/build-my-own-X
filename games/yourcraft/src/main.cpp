#include <cstdint>
#include <imgui.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>

#include "GLFW/glfw3.h"
#include "gl/context.h"
#include "gl/gl_macros.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/detail/qualifier.hpp>

#include <glad/glad.h>
#include <memory>

#include "gl/shader.h"
#include "imgui_layer.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <vector>

#include "static_for.h"


struct Cube {

    float vertice[8][3] = {
        {-1, -1, -1},
        { 1, -1, -1},
        { 1,  1, -1},
        { 1,  1, -1},
        {-1, -1,  1},
        { 1, -1,  1},
        { 1,  1,  1},
        { 1,  1,  1},
    };
    uint32_t indices[6][2][3] = {
        {},
    };
    Cube()
    {
        /*
            000 100 110 010
            001 101 111 011

            00
            10
            11
            01
         */
    }
    void init()
    {
    }
};

struct Triangle {

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    // dumy one col for multitply by matrix 4x4
    const glm::vec4 vertexs[3] = {
        {-0.5, -0.5, 0, 1},
        { 0.5, -0.5, 0, 1},
        { 0.0,  0.5, 0, 1},
    };

    uint32_t indices[3] = {0, 1, 2};


    Triangle()
    {
        // glCreateVertexArrays(1, &VAO);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        {
            GL_CALL(glGenBuffers(1, &VBO));
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            // attribs
            {
                // index 0 as the input vertex points
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *)0);
                glEnableVertexAttribArray(0);
            }
        }
        glBindVertexArray(0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        GL_CHECK_HEALTH();
    }

    void bind()
    {
        GL_CALL(glBindVertexArray(VAO));

        GL_CHECK_HEALTH();
    }

    void unbind()
    {
        glBindVertexArray(0);
    }

    ~Triangle()
    {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
    }
};

Triangle                      *tri;
static std::shared_ptr<Shader> shader;
struct VertexSpec {
    glm::vec3 vertex;
};
constexpr uint32_t max_vertex = 10000;
constexpr uint32_t max_index  = 10000;

static VertexSpec *vertexs = new VertexSpec[max_vertex];
static VertexSpec *head    = vertexs;

static uint32_t indices[max_index];
static int      indice_count;

struct Render {

    static void init()
    {
        shader = Shader::create<OpenGLShader>("../asset/shader/default.glsl");

        tri = new Triangle;
        tri->bind();

        indice_count = 0;
        for (int x = 0; x < 10000 / 3; ++x) {
            for (int i = 0; i < 3; ++i) {
                indices[x * 3 + i] = i;
            }
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tri->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, (void *)(indices), GL_STATIC_DRAW);
    }

    static void begin()
    {
        indice_count = 0;
        tri->bind();
        shader->bind();
        // todo: camera location => view, projection
        glBindBuffer(GL_ARRAY_BUFFER, tri->VBO);
    }
    static void draw_triangle(glm::vec3 pos)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) *
                              glm::rotate(glm::mat4(1.f), 0.f, {0.f, 0.f, 1.f}) *
                              glm::scale(glm::mat4(1.f), {1, 1, 1});


        for (int i = 0; i < 3; ++i) {
            head->vertex = transform * tri->vertexs[i];
            head++;
        }
        indice_count += 3;
    }
    static void end()
    {
        uint32_t size = (head - vertexs) * sizeof(VertexSpec);
        head          = vertexs;

        glBufferData(GL_ARRAY_BUFFER, size, (void *)vertexs, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tri->EBO);
        glDrawElements(GL_TRIANGLES, indice_count, GL_UNSIGNED_INT, 0);

        tri->unbind();

        GL_CHECK_HEALTH();
    }
};


int main(int, char **)
{
    printf("hello world\n");
    OpenGLContext gl_context;
    auto          window = gl_context.window;

    ImguiLayer imgui_context;
    imgui_context.init(window);

    Render::init();
    printf("hello world\n");

    while (!glfwWindowShouldClose(window)) {

        imgui_context.preupdate();
        {
            {
                if (ImGui::Begin("IMGUI")) {
                    ImGui::DragFloat4("Clear Color", glm::value_ptr(gl_context.clear_color), 0.05f, 0.f, 1.f);
                    ImGui::End();
                }
            }

            {
                Render::begin();
                Render::draw_triangle({0, 0, 0});
                // Render::draw_triangle({1, 1, 1});
                // Render::draw_triangle({-0.5, -0.5, -0.5});
                // Render::draw_triangle({+0.5, +0.5, +0.5});
                Render::end();
            }
        }

        imgui_context.postupdate();
        gl_context.update();
    }
}
