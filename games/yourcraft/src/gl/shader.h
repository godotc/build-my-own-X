#pragma once

#include "core/base.h"

#include "gl_macros.h"
#include <cstdint>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>



struct Shader {
    template <class Impl>
    static std::shared_ptr<Shader> create(const char *shader_filepath)
    {
        return std::make_shared<Impl>(shader_filepath);
    }
    // IShader<Impl>() {}

    virtual void bind()   = 0;
    virtual void unbind() = 0;

    // Unifrom matrix 4
    virtual void I1(const char *name, int32_t value)    = 0;
    virtual void UM4(const char *name, glm::mat4 value) = 0;
};

#if RENDER_OPENGL

struct OpenGLShader : public Shader {
    GLuint ID;

    OpenGLShader(const char *filename) //: IShader<OpenGLShader>()
    {
        ID                         = 0;
        std::string source         = read_file(filename);
        auto        shader_sources = preprocess(source);
        compile(shader_sources);
    }

    void bind() override
    {
        glUseProgram(ID);
    }
    void unbind() override
    {
        glUseProgram(0);
    }

    void I1(const char *name, int32_t value) override
    {
        glUniform1i(glGetUniformLocation(ID, name), value);
    }
    void UM4(const char *name, glm::mat4 value) override
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(value));
    }

  private:
    std::string                             read_file(const std::string &shader_file_path);
    std::unordered_map<GLenum, std::string> preprocess(const std::string &source);
    void                                    compile(const std::unordered_map<GLenum, std::string> &shader_sources);

    static GLenum ShaderTypeFromString(const std::string &type);
};



#endif