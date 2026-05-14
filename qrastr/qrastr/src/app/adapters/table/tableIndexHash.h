#pragma once
#include "tableIndexTypes.h"
#include <functional>

namespace std {

template<typename Tag, typename T>
struct hash<StrongIndex<Tag, T>>
{
    size_t operator()(const StrongIndex<Tag, T>& idx) const noexcept
    {
        return std::hash<T>{}(idx.value);
    }
};
}