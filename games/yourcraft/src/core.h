#pragma once

#define RENDER_OPENGL 1
// #define RENDER_VULAN 1
// #define RENDER_DIRECTX 1



#if defined(_WIN32)
    #define PLATFORM_BREAK() __debugbreak()
    #define THE_FUNCTION __FUNCSIG__
#elif defined(__clang__) || defined(__GNUC__)
    #define PLATFORM_BREAK() __builtin_trap()
    #define THE_FUNCTION __PRETTYFUNCTION__
#else
    #define PLATFORM_BREAK()
#endif


#ifndef NDEBUG
    #define YC_ASSERT(x, ...)                                                                         \
        {                                                                                             \
            if (!!!(x)) {                                                                             \
                fprintf(stderr, "Assertion Failed: %s:%d, %s\n\t", __FILE__, __LINE__, THE_FUNCTION); \
                fprintf(stderr __VA_OPT__(, ) __VA_ARGS__);                                          \
                PLATFORM_BREAK();                                                                     \
            }                                                                                         \
        }
#else
    #define YC_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
