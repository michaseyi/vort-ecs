#include "component_storage.hpp"

#ifdef COMPONENT_STORAGE_TEMPLATE_IMPL
template <typename T>
uint32_t ComponentStorage<T>::put(T component) {
    auto index = mEntries.size();
    mEntries.emplace_back(std::move(component));
    return index;
}

template <typename T>
void ComponentStorage<T>::remove(uint32_t rowIndex) {
    if (mEntries.size() > rowIndex) {
        std::swap(mEntries[rowIndex], mEntries.back());
        mEntries.pop_back();
    }
}

template <typename T>
void ComponentStorage<T>::copyFrom(uint32_t srcRow, uint32_t dstRow, ComponentStorage<T>& src) {
    while (mEntries.size() <= dstRow) {
        mEntries.emplace_back();
    }
    mEntries[dstRow] = std::move(src[srcRow]);
}

template <typename T>
T& ComponentStorage<T>::operator[](uint32_t rowIndex) {
    assert(mEntries.size() > rowIndex);
    return mEntries[rowIndex];
}

#else
#endif