#include <cstdint>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>

#include "GLFW/glfw3.h"
#include "gl/context.h"
#include "glm/gtc/type_ptr.hpp"

#include <glad/glad.h>
#include <stdio.h>

#include "gl/shader.h"
#include "imgui_layer.h"


struct Cube {
    void init()
    {
        float vertices[] = {
            // first triangle
            0.5f, 0.5f, 0.0f,   // top right
            0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, 0.5f, 0.0f,  // top left
                                // second triangle
            0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f, // bottom left
            -0.5f, 0.5f, 0.0f   // top left
        };
    }
};

struct Triangle {

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    const float points[4][3] = {
        {-0.5, -0.5, 0},
        { 0.5, -0.5, 0},
        { 0.0,  0.5, 0},
        { 0.5,  0.5, 0},
    };

    uint32_t indices[2][3] = {
        {0, 1, 2},
        {2, 1, 3}
    };


    Triangle()
    {
        // glCreateVertexArrays(1, &VAO);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        {
            GL_CALL(glGenBuffers(1, &VBO));
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(points), (float *)points, GL_STATIC_DRAW);

            // index 0 as the input vertex points
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *)0);
            glEnableVertexAttribArray(0);
        }
        glBindVertexArray(0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        GL_CHECK_HEALTH();
    }

    void update()
    {
        GL_CALL(glBindVertexArray(VAO));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(uint32_t), GL_UNSIGNED_INT, 0);
        // glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(0);
        GL_CHECK_HEALTH();
    }

    ~Triangle()
    {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
    }
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
