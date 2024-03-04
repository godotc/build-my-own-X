#include "shader.h"

#include <cassert>
#include <cstdio>
#include <fstream>

std::string OpenGLShader::read_file(const std::string &shader_file_path)
{

    std::string   result;
    std::ifstream file(shader_file_path, std::ios::in | std::ios::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        auto msg = std::string("Failed to load shader from file: {}") + shader_file_path;
        fprintf(stderr, "%s", msg.c_str());
        assert(0);
        // return result;
    }
    if (!file) {
        auto msg = "Failed to load shader from file:" + shader_file_path;
        assert(0);
        fprintf(stderr, "%s", msg.c_str());
        // return result;
    }

    file.seekg(0, std::ios::end); // to end
    result.resize(file.tellg());  // size
    file.seekg(0, std::ios::beg); // to begin
    file.read(result.data(), result.size());
    file.close();

    return result;
}
std::unordered_map<GLenum, std::string> OpenGLShader::preprocess(const std::string &source)
{
    std::unordered_map<GLenum, std::string> shader_sources;

    const char  *type_token     = "#type";
    const size_t type_token_len = strlen(type_token);
    size_t       pos            = source.find(type_token, 0);

    static const char *eol_flag =
#if _WIN32
        "\r\n";
#elif __linux__
        "\n";
#endif


    while (pos != std::string ::npos)
    {

        // get the type string
        size_t eol = source.find_first_of(eol_flag, pos);
        assert(eol != std::string::npos);

        size_t      begin = pos + type_token_len + 1;
        std::string type  = source.substr(begin, eol - begin);

        GLenum shader_type = ShaderTypeFromString(type);
        assert(shader_type);

        // get the shader content range
        size_t next_line_pos = source.find_first_not_of(eol_flag, eol);
        // next shader content begin
        pos = source.find(type_token, next_line_pos);

        auto codes = source.substr(next_line_pos, pos - (next_line_pos == std::string ::npos ? source.size() - 1 : next_line_pos));

        auto [_, Ok] = shader_sources.insert({(unsigned int)shader_type, codes});
        //        HZ_CORE_INFO("{} \n {}", _->first, _->second);
        YC_ASSERT(Ok, "Failed to insert this shader source");
    }

    return shader_sources;
}
void OpenGLShader::compile(const std::unordered_map<GLenum, std::string> &shader_sources)
{
    GLuint program;
    GL_CALL(program = glCreateProgram());
    YC_ASSERT((shader_sources.size() <= 2 && shader_sources.size() > 0), "Only 2 shaders for now/ 0 Shaders for compiling");

    // TODO: use array for fixed numbers of shaders
    std::vector<GLuint> shaders;
    shaders.reserve(shader_sources.size());

    for (auto &[shader_type, source] : shader_sources)
    {
        GLuint shader_handle;
        // vertex Shader
        GL_CALL(shader_handle = glCreateShader(shader_type));
        const GLchar *src = source.c_str();
        GL_CALL(glShaderSource(shader_handle, 1, &src, nullptr));
        GL_CALL(glCompileShader(shader_handle));

        GLint success;
        GL_CALL(glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success));
        if (!success) {
            GLint max_len = {0};
            glGetShaderiv(shader_type, GL_INFO_LOG_LENGTH, &max_len);
            std::vector<GLchar> log(max_len);
            GL_CALL(glGetShaderInfoLog(shader_handle, max_len, &max_len, log.data()));

            glDeleteShader(shader_handle);

            fprintf(stderr, "%s", log.data());
            YC_ASSERT(false, "shader compilation failed!");

            break;
        }

        shaders.push_back(shader_handle);
    }

    for (auto shader : shaders) {
        // GL_CALL(
        glAttachShader(program, shader);
        //);
    }
    GL_CALL(glLinkProgram(program));

    GLint bIsLinked;
    GL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &bIsLinked));
    if (!bIsLinked) {
        GLint max_len = {0};
        GL_CALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_len));
        std::vector<GLchar> log(max_len);
        GL_CALL(glGetProgramInfoLog(program, max_len, &max_len, &log[0]));

        glDeleteProgram(program);
        for (auto shader : shaders) {
            GL_CALL(glDeleteShader(shader));
        }

        fprintf(stderr, "Program Linkage: %s", log.data());
        YC_ASSERT(false, "shader compilation failed!");
        return;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    for (auto shader : shaders) {
        GL_CALL(glDeleteShader(shader));
    }

    this->ID = program;
}
GLenum OpenGLShader::ShaderTypeFromString(const std::string &type)
{
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    else if (type == "fragment" || type == "pixel")
        return GL_FRAGMENT_SHADER;

    fprintf(stderr, "Unknown shader type!");
    assert(0);
    return 0;
}