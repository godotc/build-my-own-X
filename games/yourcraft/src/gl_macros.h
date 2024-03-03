//
// Created by nono on 9/20/23.
//

#ifndef HAZEL_GL_MACROS_H
#define HAZEL_GL_MACROS_H


#pragma once

#include <glad/glad.h>
#include <iostream>
#include <string>


namespace gltk {

inline void GLClearError()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

#define GL_GETERROR_STRING(error) ""


inline bool GLLogCall(const char *function, const char *file, int line)
{
    while (GLenum error = glGetError())
    {
        std::string error_str;
        switch (error)
        {
        case GL_INVALID_ENUM:
            error_str = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error_str = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error_str = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error_str = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error_str = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error_str = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error_str = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }

        std::cerr << "[OpenGL Error] (" << error_str << ":" << GL_GETERROR_STRING(error) << ")"
                  << ": " << function << " " << file << ":" << line
                  << std::endl;
        return false;
    }
    return true;
}


// inline void warnning()
//{
// #pragma message("Warning: Using GL_CALL with glEnd()")
// }

} // namespace gltk

#if defined(_WIN32)
    #define PLATFORM_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    #define PLATFORM_BREAK() __builtin_trap()
#else
    #define PLATFORM_BREAK()
#endif

#ifndef NDEBUG
    #define __GL_ASSERT(x) \
        if (!(x))          \
        PLATFORM_BREAK()
#else
    #define __GL_ASSERT(x) (x)
#endif



#ifdef NDEBUG
    #define GL_CALL(x) x
#else
    #define GL_CALL(x)                                            \
        do {                                                      \
            gltk::GLClearError();                                 \
            x;                                                    \
            __GL_ASSERT(gltk::GLLogCall(#x, __FILE__, __LINE__)); \
        } while (0)
#endif

#ifdef NDEBUG
    #define GL_CHECK_HEALTH()
#else
    #define GL_CHECK_HEALTH()                                           \
        do {                                                            \
            __GL_ASSERT(gltk::GLLogCall(__func__, __FILE__, __LINE__)); \
        } while (0)
#endif



#endif // HAZEL_GL_MACROS_H