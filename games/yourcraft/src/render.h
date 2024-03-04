#pragma once
#include "gl/shader.h"
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <memory>
#include <vector>



struct Vertex {
};


struct Triangle {

    GLuint VAO;

    Triangle()
    {
        glCreateBuffers(1, &VAO);
    }
};


struct Cube {
};

template <class Impl>
struct VertexArray {
};


struct BufferLayout {
    enum ElemType
    {
        float32,
        int32,
    };
    enum StepMode
    {
        vertex
    };
    struct Attribute {
        unsigned int shader_location;
        unsigned int offset;
        ElemType     Type;
        unsigned int Num;
    };

    std::vector<Attribute> attributes;
    unsigned int           stride;
    StepMode               step_mode;
};

void a(BufferLayout l)
{
}


template <class Impl>
struct RenderAPI {
};

struct OpenGLRenderAPI : public RenderAPI<OpenGLRenderAPI> {
    void Init()
    {
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    void SetClearColor(const glm::vec4 &color)
    {
        glClearColor(color.x, color.y, color.z, color.w);
    }
    void Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};


struct Render {
};


class Render3D : public Render
{

    static struct {
        glm::mat4 ViewProjectionMatrix;
    } m_SceneData;

  public:
    static void BeginSecene(const glm::mat4 &view_proj_mat)
    {
        m_SceneData.ViewProjectionMatrix = view_proj_mat;
    }
    static void EndScene() {}
    template <class ShaderImpl>
    static void Submit(
        std::shared_ptr<IShader<ShaderImpl>> &shader,
        std::shared_ptr<VertexArray>         &vertex_array,
        const glm::mat4                      &transform)
    {
        shader->bind();
        shader->UM4("u_ViewProjection", m_SceneData.ViewProjectionMatrix);
        shader->UM4("u_Transform", transform);
        vertex_array->bind();
    }
};
