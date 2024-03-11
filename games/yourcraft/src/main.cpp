#include <cstdint>
#include <imgui.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>

#include "GLFW/glfw3.h"
#include "gl/context.h"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <glad/glad.h>

#include "gl/shader.h"
#include "imgui_layer.h"

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

    const float vertexs[3][3] = {
        {-0.5, -0.5, 0},
        { 0.5, -0.5, 0},
        { 0.0,  0.5, 0},
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
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), (float *)vertexs, GL_STATIC_DRAW);

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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        GL_CHECK_HEALTH();
    }

    void bind()
    {
        GL_CALL(glBindVertexArray(VAO));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        GL_CHECK_HEALTH();
    }

    void draw()
    {
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(uint32_t), GL_UNSIGNED_INT, 0);
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


struct Render {

    static DrawTriangle();
};


int main(int, char **)
{
    OpenGLContext gl_context;
    auto          window = gl_context.window;

    ImguiLayer imgui_context;
    imgui_context.init(window);

    auto     shader = Shader::create("../asset/shader/default.glsl");
    Triangle tri;

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
                shader->bind();
                tri.update();
            }
        }

        imgui_context.postupdate();
        gl_context.update();
    }
}
