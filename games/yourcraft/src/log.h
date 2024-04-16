#pragma once


#include "glm/ext/vector_float4.hpp"
#include <iostream>
#include <optional>
#include <rang.hpp>
#include <span>

template <class T>
inline std::ostream &operator<<(std::ostream &out, const std::optional<T> &v)
{
    if (v.has_value()) {
        out << v.value();
    }
    else {
        out << "[None]";
    }
    return out;
}

template <class T>
inline std::ostream &operator<<(std::ostream &out, std::span<T> v)
{
    out << '{';
    for (auto const &elem : v.subspan(0, v.size() - 1)) {
        out << v << ", ";
    }
    out << v.back();
    out << '}';
}


struct debug {
    debug operator=(debug &&) = delete;
#ifndef NDEBUG
    debug() { std::cout << rang::style::underline << rang::fg::cyan; }
    debug &operator,(auto val)
    {
        std::cout << val;
        return *this;
    }
    ~debug()
    {
        std::cout << '\n'
                  << rang::style::reset
                  << rang::fg::reset;
    }
#else
    debug &operator,(auto val) { return *this; }
#endif
};
// static auto dbg=debug{};
