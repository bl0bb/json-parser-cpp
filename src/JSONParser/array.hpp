#ifndef ARRAY_H
#define ARRAY_H

#include <cstddef>

template<typename T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept {
    return N;
}

#endif