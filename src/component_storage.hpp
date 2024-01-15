#pragma once
#include <cassert>
#include <iostream>
#include <vector>

template <typename T>
class ComponentStorage {
    static_assert(std::is_default_constructible_v<T>);

public:
    u_int32_t put(T component);

    void remove(u_int32_t rowIndex);

    void copyFrom(u_int32_t srcRow, u_int32_t dstRow, ComponentStorage<T>& src);

    T& operator[](u_int32_t rowIndex);

private:
    std::vector<T> mEntries;
};

#define COMPONENT_STORAGE_TEMPLATE_IMPL
#include "component_storage.cpp"
#undef COMPONENT_STORAGE_TEMPLATE_IMPL
