#include "component_storage.hpp"

#ifdef COMPONENT_STORAGE_TEMPLATE_IMPL
template <typename T> template <typename _T> u_int32_t ComponentStorage<T>::put(_T&& component) {
    assert((std::is_same_v<std::decay_t<T>, std::decay_t<_T>>));
    return _put(std::forward<_T&>(component));
}

template <typename T> void ComponentStorage<T>::remove(u_int32_t rowIndex) {
    if (mEntries.size() > rowIndex) {
        std::swap(mEntries[rowIndex], mEntries.back());
        mEntries.pop_back();
    }
}

template <typename T> void ComponentStorage<T>::copyFrom(u_int32_t srcRow, u_int32_t dstRow, ComponentStorage<T>& src) {
    while (mEntries.size() <= dstRow) {
        mEntries.emplace_back();
    }
    mEntries[dstRow] = src[srcRow];
}

template <typename T> T& ComponentStorage<T>::operator[](u_int32_t rowIndex) {
    assert(mEntries.size() > rowIndex);
    return mEntries[rowIndex];
}

template <typename T> u_int32_t ComponentStorage<T>::_put(T& component) {
    auto index = mEntries.size();
    mEntries.emplace_back(component);
    return index;
}
#else
#endif