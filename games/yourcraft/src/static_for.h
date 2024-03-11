

#pragma once

#include <type_traits>
#include <utility>

template <int StartIndex, int Upperbound, class Fn, class... Arg>
void static_for(Fn func, Arg... args)
{
    if constexpr (StartIndex < Upperbound) {
        func(std::integral_constant<int, StartIndex>{}, std::forward<Arg>(args)...);
        static_for<StartIndex + 1, Upperbound>(func, std::forward<Arg>(args)...);
    }
}
