#include <cstddef>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <imgui.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


#include "core/base.h"
#include "delegate.h"

#include "GLFW/glfw3.h"
#include "gl/context.h"
#include "gl/gl_macros.h"

#include <glad/glad.h>
#include <memory>

#include "gl/shader.h"
#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/vector_float4.hpp"
#include "imgui_layer.h"

#include "glm/exponential.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>


#define GLM_ENABLE_EXPERIMENTAL
// #include <glm/gtx/quaternion.hpp>

void print(auto text, glm::vec3 v) { printf("%s: (%f, %f, %f)\n", text, v.x, v.y, v.z); };
void print(auto text, glm::vec2 v) { printf("%s: (%f, %f)\n", text, v.x, v.y); };
void print(glm::vec2 v) { printf(" (%f, %f)\n", v.x, v.y); };
void print(glm::vec3 v) { printf(" (%f, %f, %f)\n", v.x, v.y, v.z); };

static std::shared_ptr<Shader> shader;
struct VertexSpec {
    glm::vec3 vertex;
    glm::vec4 color = glm::vec4(1.f);

    static void SetupVertexAttribs()
    {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void *)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void *)(sizeof(float) * 3));
    }
};
constexpr uint32_t max_vertex = 10000;
constexpr uint32_t max_index  = 10000;

static VertexSpec  vertexes[max_vertex];
static VertexSpec *vertexes_head = vertexes;

static uint32_t  indices[max_index];
static uint32_t *indices_head = indices;

static GLuint VAO, VBO, EBO;
uint32_t      vertex_count = 0;
uint32_t      index_count  = 0;


namespace Input {

static int keyboards[1024];
static int MouseButtons[32];
bool       IskeyPressed(GLenum key) { return keyboards[key]; }
bool       IsMouseButtonPressed(GLenum key) { return MouseButtons[key]; }
glm::vec2  GetMousePos()
{
    double w, h;
    glfwGetCursorPos(OpenGLContext::Get().window, &w, &h);
    return {w, h};
}

} // namespace Input


struct {
    const glm::vec4 vertices[8] = {
  // Front face
        {-0.5f, -0.5f,  0.5f, 1.f}, // Bottom-left
        { 0.5f, -0.5f,  0.5f, 1.f}, // Bottom-right
        { 0.5f,  0.5f,  0.5f, 1.f}, // Top-right
        {-0.5f,  0.5f,  0.5f, 1.f}, // Top-left

  // Back face
        {-0.5f, -0.5f, -0.5f, 1.f}, // Bottom-left
        { 0.5f, -0.5f, -0.5f, 1.f}, // Bottom-right
        { 0.5f,  0.5f, -0.5f, 1.f}, // Top-right
        {-0.5f,  0.5f, -0.5f, 1.f}  // Top-left
    };

    const float size = 0.5f;

    const std::vector<unsigned int> indices = {
        // Front face
        0, 1, 2, // Triangle 1
        2, 3, 0, // Triangle 2
        // Right face
        1, 5, 6, // Triangle 1
        6, 2, 1, // Triangle 2
        // Back face
        5, 4, 7, // Triangle 1
        7, 6, 5, // Triangle 2
        // Left face
        4, 0, 3, // Triangle 1
        3, 7, 4, // Triangle 2
        // Top face
        3, 2, 6, // Triangle 1
        6, 7, 3, // Triangle 2
        // Bottom face
        4, 5, 1, // Triangle 1
        1, 0, 4  // Triangle 2
    };
} Cube;


struct {
    const glm::vec4 vertices[3] = {
        {-0.5f, -0.5f, 0.5f, 1.f},
        {0.5f, -0.5f, 0.5f, 1.f},
        {0, 1 - glm::pow(0.5, 2), 0.f, 1.f},
    };
    const std::vector<unsigned int> indices = {0, 1, 2};
} Triangle;



static constexpr glm::vec3 world_up = glm::vec3(0, 1, 0);
struct Camera {

    glm::vec3 pos = glm::vec3(0, 0, 0);

    glm::vec3 camera_forward = glm::vec3(0, 0, -1);
    glm::vec3 camera_upward  = world_up;
    glm::vec3 camera_right   = glm::cross(camera_forward, world_up);

    float yaw   = -90.f;
    float pitch = 0.f;
    float roll  = 0.f;

    float sensitivity = 5.f;

    float speed = 5.f;

    glm::vec2 window_size = {800, 600};

    glm::vec2 last_mouse_pos = {};


    bool bMouseAndKeyBoard = true;

    Camera()
    {
        update_directions();
    }


    glm::mat4 GetViewProjectionMatrix() const
    {
        return
            // TODO: no need to calc the perspective matrix every time
            glm::perspective(glm::radians(45.f), window_size.x / (float)window_size.y, 0.1f, 100.f) *
            GetViewMatrix();
    }

    glm::mat4 GetViewMatrix() const
    {
        // return glm::lookAt(pos, pos + camera_forward, world_up); // Should not be the world's up
        return glm::lookAt(pos, pos + camera_forward, camera_upward);
    }

    void OnUpdate(float dt)
    {

        // p("right", right);
        // p("world up", world_up);
        // printf("\t");
        // p("up", up);
        // printf("\n");
        // p("forward", camera_forward_dir);
        // return;

        if (Input::IskeyPressed(GLFW_KEY_W))
        {
            pos += camera_forward * speed * dt;
        }
        if (Input::IskeyPressed(GLFW_KEY_S)) {
            pos -= camera_forward * speed * dt;
        }
        if (Input::IskeyPressed(GLFW_KEY_A)) {
            pos -= camera_right * speed * dt;
        }
        if (Input::IskeyPressed(GLFW_KEY_D)) {
            pos += camera_right * speed * dt;
        }
        if (Input::IskeyPressed(GLFW_KEY_Q)) {
            pos += world_up * speed * dt;
        }
        if (Input::IskeyPressed(GLFW_KEY_E)) {
            pos -= world_up * speed * dt;
        }

        OnMouseMove(dt);
        update_directions();
    }

    void OnMouseMove(float dt)
    {

        if (!Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            return;
        }

        glm::vec2 mouse_pos = Input::GetMousePos();

        auto offset = (mouse_pos - last_mouse_pos) * sensitivity * dt;
        // if (offset.x < f || offset.y < 0.1f) {
        //     return;
        // }
        last_mouse_pos = mouse_pos;

        // print("mouse pos", mouse_pos);

        // printf("offset: %f, %f\n", offset.x, offset.y);

        yaw += offset.x;
        pitch -= offset.y;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // the x and z influenced by the pitch
        // imagine the yaw is -90(the right forward 正前方), pitch is 45 (roll up 45degree 向上抬45度)
        //   the length in x and z be 1 as default (x 和 z 本该是1)
        //  but roll up make it projection on x and z reduces (45度仰角投影在x和z上的长度小于0度仰角，由pitch控制)
        // the y did not be affected: by the roll-up degree (y即高度，只跟仰角相关)  -> NO, up is y
        // Above logic from euler angles(根据欧拉角来的，先x,z再y, 反过来也行？ 不过人是主要左右转再上下看的....)


        // float     t = glm::cos(glm::radians(pitch));
        // glm::vec3 dir;
        // dir = {glm::cos(glm::radians(yaw)) * t,
        //        glm::sin(glm::radians(pitch)),
        //        glm::sin(glm::radians(yaw)) * t};
        // // glm::quat q(roll, pitch, yaw);
        // camera_forward_dir = glm::normalize(dir);
    }

    void OnWindowFocusChanged(bool bFocused)
    {
    }

    void update_directions()
    {
        float     t = glm::cos(glm::radians(pitch));
        glm::vec3 dir;
        dir = {glm::cos(glm::radians(yaw)) * t,
               glm::sin(glm::radians(pitch)),
               glm::sin(glm::radians(yaw)) * t};
        // glm::quat q(roll, pitch, yaw);
        camera_forward = glm::normalize(dir);
        camera_right   = glm::cross(world_up, camera_forward);
        camera_upward  = glm::cross(camera_forward, camera_right);
    }

    void OnMouseButtonEvent(unsigned int btn, unsigned int action, unsigned int mods)
    {
        if (btn == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS) {
                last_mouse_pos = Input::GetMousePos();
            }
            else {
            }
        }
    }
};



struct Render {

    static void init()
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        {
            // VBO
            {
                GL_CALL(glGenBuffers(1, &VBO));
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(VertexSpec) * max_vertex, nullptr, GL_STATIC_DRAW);

                // // attribs
                // {
                //     // index 0 as the input vertex points
                //     glEnableVertexAttribArray(0);
                //     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *)0);
                // }
                VertexSpec::SetupVertexAttribs();
            }


            // EBO
            {
                glGenBuffers(1, &EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * max_index, nullptr, GL_STATIC_DRAW);
            }
        }

        glBindVertexArray(0);

        GL_CHECK_HEALTH();


        shader = Shader::create<OpenGLShader>("../asset/shader/default.glsl");

        glBindBuffer(GL_ARRAY_BUFFER, VAO);
        // In the Triangle constructor:
    }

    static void begin(const Camera &camera)
    {
        vertex_count = 0;
        index_count  = 0;

        glm::mat4 view_projection = camera.GetViewProjectionMatrix();
        // glm::mat4 view_projection = camera.GetViewMatrix();

        shader->bind();
        shader->UM4("u_ViewProjection", view_projection);

        glBindVertexArray(VAO);
        // todo: camera location => view, projection
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
    }
    static void draw_triangle(glm::vec3 pos, glm::vec3 scale = {1, 1, 1})
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) *
                              glm::rotate(glm::mat4(1.f), 0.f, {0.f, 0.f, 1.f}) *
                              glm::scale(glm::mat4(1.f), scale);


        size_t size = Triangle.indices.size();
        for (int i = 0; i < size; ++i) {
            *indices_head = Triangle.indices[i] + vertex_count;
            indices_head++;
        }

        for (int i = 0; i < 3; ++i)
        {
            vertexes_head->vertex = transform * Triangle.vertices[i];
            vertexes_head++;
        }
        vertex_count += 3;
    }

    static void draw_cube(glm::vec3 pos, glm::vec3 scale, glm::vec4 color = glm::vec4(1.f))
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) *
                              glm::rotate(glm::mat4(1.f), 0.f, {0.f, 0.f, 1.f}) *
                              glm::scale(glm::mat4(1.f), scale);

        size_t size = Cube.indices.size();
        for (int i = 0; i < size; ++i) {
            *indices_head = Cube.indices[i] + vertex_count;
            indices_head++;
        };

        for (int i = 0; i < 8; ++i) {
            vertexes_head->vertex = transform * Cube.vertices[i];
            vertexes_head->color  = color;
            vertexes_head++;
        }
        vertex_count += 8;
    }

    static void end()
    {
        uint32_t vertexes_size = (vertexes_head - vertexes) * sizeof(VertexSpec);
        // uint32_t size = (uint8_t *)head - (uint8_t *)vertexes;
        vertexes_head = vertexes;
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexes_size, (void *)vertexes);


        uint32_t indices_count = indices_head - indices;
        indices_head           = indices;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices_count * sizeof(uint32_t), indices);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices_count, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        GL_CHECK_HEALTH();
    }
};


int main(int, char **)
{
    printf("hello world\n");
    auto gl_context = OpenGLContext::Get();
    auto window     = gl_context.window;


    ImguiLayer imgui_context;
    imgui_context.init(window);

    Render::init();


    printf("hello world\n");

    glm::vec3 pos = {0, 0, -1}, direction = {0, 0, 0};


    Camera camera;

    glfwSetKeyCallback(gl_context.window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            Input::keyboards[key] = 1;
        }
        else
        {
            Input::keyboards[key] = 0;
        }
    });
    static MulticastDelegate<glm::vec2> OnMouseMove;
    glfwSetCursorPosCallback(gl_context.window, [](GLFWwindow *window, double xpos, double ypos) {
        OnMouseMove.Broadcast({xpos, ypos});
    });

    static MulticastDelegate<GLenum /*button*/, GLenum /*action*/, GLenum /*mods*/> OnMouseButtonEvent;
    OnMouseButtonEvent.Add(&camera, &Camera::OnMouseButtonEvent);
    glfwSetMouseButtonCallback(gl_context.window, [](GLFWwindow *window, int button, int action, int mods) {
        Input::MouseButtons[button] = action == GLFW_PRESS ? 1 : 0;
        OnMouseButtonEvent.Broadcast(button, action, mods);
    });

    static MulticastDelegate<bool> OnWindowFocusChanged;
    OnWindowFocusChanged.Add(&camera, &Camera::OnWindowFocusChanged);
    glfwSetWindowFocusCallback(window, [](GLFWwindow *window, int focused) {
        OnWindowFocusChanged.Broadcast(focused == GLFW_TRUE);
    });

    // OnWindowFocusChanged.AddWeak(&camera, ())


    double last_time = glfwGetTime();
    double time      = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        time      = glfwGetTime();
        double dt = time - last_time;
        last_time = time;
        // printf("%f\n", dt);

        imgui_context.preupdate();
        {
            {
                if (ImGui::Begin("IMGUI")) {
                    ImGui::DragFloat4("Clear Color", glm::value_ptr(gl_context.clear_color), 0.05f, 0.f, 1.f);
                    ImGui::DragFloat3("Pos", glm::value_ptr(camera.pos), 0.3);
                    ImGui::DragFloat3("Forwards", glm::value_ptr(camera.camera_forward));
                    if (ImGui::Button("Rest Camera")) {
                        camera.pos            = glm::vec3(0, 0, -10);
                        camera.camera_forward = glm::vec3(0, 0, 1);
                        camera.yaw            = -90.f;
                        camera.pitch          = 0.f;
                        camera.roll           = 0.f;
                    }
                    {
                        ImGui::DragFloat("Yaw", &camera.yaw);
                        ImGui::DragFloat("Pitch", &camera.pitch);
                        ImGui::DragFloat("Roll", &camera.roll);
                    }
                    ImGui::End();
                }
            }
        }

        {

            int w, h;
            glfwGetWindowSize(window, &w, &h);

            camera.window_size = {w, h};
            camera.OnUpdate(dt);
        }

        {
            Render::begin(camera);
            Render::draw_triangle({0, 0, -1}, glm::vec3(5));
            Render::draw_triangle({1, 1, 1});
            Render::draw_triangle({-0.5, -0.5, -0.5});
            Render::draw_triangle({+0.5, +0.5, +0.5});

            glm::vec3 pos = {-10, 0, -10};
            for (int i = 0; i < 10; ++i)
            {
                for (int j = 0; j < 10; ++j) {
                    auto p = pos;
                    p.x += Cube.size * i;
                    p.y += Cube.size * j;
                    auto color = glm::vec4(
                        i / 10.f, j / 10.f,
                        0, 1);
                    Render::draw_cube(p, glm::vec3(1), color);
                }
            }

            Render::draw_cube({0, 0, 0}, {5, 1, 5}, {1, 0, 0, 1});
            Render::end();
            glPointSize(15.f);
            glBegin(GL_POINTS);
            glVertex3f(0, 0, 0);
            glVertex3f(1, 0, 0);
            glEnd();
        }

        imgui_context.postupdate();
        gl_context.update();
    }
}
