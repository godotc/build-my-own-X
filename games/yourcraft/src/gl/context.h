#pragma once
#include "gl/gl_macros.h"
#include "glm/ext/vector_float4.hpp"
#include <GLFW/glfw3.h>
#include <glad/glad.h>



struct OpenGLContext {
    GLFWwindow *window;
    glm::vec4   clear_color;

    OpenGLContext()
    {
        init();
    }

    ~OpenGLContext()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void init();
    void config()
    {
        glEnable(GL_DEPTH_TEST);
        glfwSwapInterval(1); // Enable vsync
    }

    void update()
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_HEALTH();
    }
};