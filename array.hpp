#ifndef DEOHAYER_AP_ARRAY_HPP
#define DEOHAYER_AP_ARRAY_HPP

#include "core.hpp"
#include <memory>   // std::unique_ptr
#include <stdlib.h> // malloc, realloc, free

namespace ap
{
namespace library
{

template <typename T>
using array = std::unique_ptr<T[], void (*)(void*)>;

template <typename T>
array<T> array_alloc(index_t count)
{
    return array<T>{reinterpret_cast<T*>(std::malloc(sizeof(T) * count)), &std::free};
}

template <typename T>
array<T>& array_realloc(array<T>& arr, index_t count)
{
    T* raw_ptr = arr.release();
    arr.reset(reinterpret_cast<T*>(std::realloc(raw_ptr, sizeof(T) * count)));
    return arr;
}

template <typename T>
array<T> array_null()
{
    return array<T>{reinterpret_cast<T*>(NULL), &std::free};
}

} // namespace library
} // namespace ap

#endif
