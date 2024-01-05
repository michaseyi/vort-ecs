#include "erased_component.hpp"

#ifdef ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL
template <typename T> ComponentStorage<T>* ErasedComponentStorage::cast() {
    return reinterpret_cast<ComponentStorage<T>*>(mRaw);
}

template <typename T> ErasedComponentStorage ErasedComponentStorage::create() {
    auto componentStorage = new ComponentStorage<T>();
    auto deleter = [](void* rawPtr) { delete reinterpret_cast<ComponentStorage<T>*>(rawPtr); };

    auto cloner = [](ErasedComponentStorage& src, ErasedComponentStorage& retval) {
        auto newComponentStorage = new ComponentStorage<T>();
        retval = std::move(ErasedComponentStorage{reinterpret_cast<void*>(newComponentStorage), src.deleter, src.cloner,
                                                  src.copier, src.remover});
    };

    auto copy = [](ErasedComponentStorage& dst, u_int32_t srcRow, u_int32_t dstRow, ErasedComponentStorage& src) {
        ComponentStorage<T>& destination = *dst.cast<T>();
        ComponentStorage<T>& source = *src.cast<T>();
        destination.copyFrom(srcRow, dstRow, source);
    };

    auto remove = [](ErasedComponentStorage& src, u_int32_t row) {
        ComponentStorage<T>& source = *src.cast<T>();
        source.remove(row);
    };

    return ErasedComponentStorage{reinterpret_cast<void*>(componentStorage), deleter, cloner, copy, remove};
}

#else
ErasedComponentStorage::ErasedComponentStorage(ErasedComponentStorage&& rhs) {

    this->~ErasedComponentStorage();
    deleter = rhs.deleter;
    cloner = rhs.cloner;
    remover = rhs.remover;
    copier = rhs.copier;
    mRaw = rhs.mRaw;
    rhs.mRaw = nullptr;
}

ErasedComponentStorage& ErasedComponentStorage::operator=(ErasedComponentStorage&& rhs) {
    this->~ErasedComponentStorage();
    deleter = rhs.deleter;
    cloner = rhs.cloner;
    remover = rhs.remover;
    copier = rhs.copier;
    mRaw = rhs.mRaw;
    rhs.mRaw = nullptr;
    return *this;
}

ErasedComponentStorage::~ErasedComponentStorage() {
    if (mRaw) {
        deleter(mRaw);
    }
}

void ErasedComponentStorage::copyFrom(u_int32_t srcRow, u_int32_t dstRow, ErasedComponentStorage& src) {
    copier(*this, srcRow, dstRow, src);
}

void ErasedComponentStorage::remove(u_int32_t rowIndex) { remover(*this, rowIndex); }

void ErasedComponentStorage::cloneTo(ErasedComponentStorage& retval) { cloner(*this, retval); }
#endif