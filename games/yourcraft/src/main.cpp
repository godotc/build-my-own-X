#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>

#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <glad/glad.h>
#include <stdio.h>

#include "shader.h"

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


struct Triangle {

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint IBO = 0;



    void init()
    {
        // glCreateVertexArrays(1, &VAO);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        {

            glGenBuffers(1, &VBO);
            float points[3][3] = {
                {-0.5, -0.5, 0},
                { 0.5, -0.5, 0},
                { 0.0,  0.5, 0},
            };
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

            // index 0 as the input vertex points
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 9, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *)0);

            glGenBuffers(1, &IBO);
            float indices[] = {0, 1, 2};
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        }
        glBindVertexArray(0);
    }

    void bind()
    {
        glBindVertexArray(VAO);
    }
    void unbind()
    {
        glBindVertexArray(0);
    }

    void update()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
    }

    ~Triangle()
    {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &IBO);
        glDeleteVertexArrays(1, &VAO);
    }
};


static void gl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{

    // FIXME: avoid noise, too many ouput info
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    using std::cout, std::endl;
    cout << "---------------------opengl-callback-start------------" << endl;
    cout << "message: " << message << endl;
    cout << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        cout << "OTHER";
        break;
    }
    cout << endl;

    cout << "id: " << id << endl;
    cout << "severity: ";
    switch (severity) {
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        cout << "NOTIFICATION";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        cout << "HIGH";
        break;
    }
    cout << endl;
    cout << "---------------------opengl-callback-end--------------" << endl;
}

int main(int, char **)
{
    if (GLFW_FALSE == glfwInit()) {
        return -1;
    }

    glfwSetErrorCallback(glfw_error_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
        return -1;
    }

    // Notice: this is a specific driver extension
    glEnable(GL_DEBUG_OUTPUT);
    if (glDebugMessageCallback != nullptr) {
        glDebugMessageCallback(gl_message_callback, nullptr);
        printf("Bound GL debug callback successfully\n");
    }
    else {
        printf("glDebugMessageCallback is nullptr. Maybe your driver is not supportting this extionsion!\n");
    }


    // imgui inits
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    {
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
        // io.ConfigViewportsNoAutoMerge = true;
        // io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    glfwSwapInterval(1); // Enable vsync
    glm::vec4 clear_color{0, 0, 0, 1};
    Triangle  tri;
    tri.init();
    auto shader = Shader::create("../asset/shader/default.glsl");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // imgui begins
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }


        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        {

            if (ImGui::Begin("IMGUI")) {
                ImGui::DragFloat4("Clear Color", glm::value_ptr(clear_color), 0.05f, 0.f, 1.f);
                ImGui::End();
            }

            // TODO: move
            {
                int w, h;
                glfwGetFramebufferSize(window, &w, &h);
                glViewport(0, 0, w, h);
            }

            shader->bind();
            tri.bind();
            tri.update();
            // tri.unbind();

            // glPointSize(20.f);

            // glBegin(GL_POINTS);
            // glVertex3f(1, 1, 1);
            // glVertex3f(2, 2, 2);
            // glVertex3f(0.5, 0.5, 0.5);
            // glEnd();
        }

        // imgui ends
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow *backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }
        }

        glfwSwapBuffers(window);
    }

    // IMGUI Destroy
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
