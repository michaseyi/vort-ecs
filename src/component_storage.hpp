#pragma once
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

template <typename T>
class ComponentStorage {
    static_assert(std::is_default_constructible_v<T>);

public:
    uint32_t put(T component);

    void remove(uint32_t rowIndex);

    void copyFrom(uint32_t srcRow, uint32_t dstRow, ComponentStorage<T>& src);

    T& operator[](uint32_t rowIndex);

private:
    std::vector<T> mEntries;
};

#define COMPONENT_STORAGE_TEMPLATE_IMPL
#include "component_storage.cpp"
#undef COMPONENT_STORAGE_TEMPLATE_IMPL
