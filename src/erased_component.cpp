#include "erased_component.hpp"

#ifdef ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL
template <typename T>
ComponentStorage<T>* ErasedComponentStorage::cast() {
    return reinterpret_cast<ComponentStorage<T>*>(mRaw);
}

template <typename T>
ErasedComponentStorage ErasedComponentStorage::create() {
    auto componentStorage = new ComponentStorage<T>();
    auto deleter = [](void* rawPtr) { delete reinterpret_cast<ComponentStorage<T>*>(rawPtr); };

    auto cloner = [](ErasedComponentStorage& src, ErasedComponentStorage& retval) {
        auto newComponentStorage = new ComponentStorage<T>();
        retval = ErasedComponentStorage{reinterpret_cast<void*>(newComponentStorage), src.mDeleter, src.mCloner,
                                        src.mCopier, src.mRemover};
    };

    auto copy = [](ErasedComponentStorage& dst, uint32_t srcRow, uint32_t dstRow, ErasedComponentStorage& src) {
        ComponentStorage<T>& destination = *dst.cast<T>();
        ComponentStorage<T>& source = *src.cast<T>();
        destination.copyFrom(srcRow, dstRow, source);
    };

    auto remove = [](ErasedComponentStorage& src, uint32_t row) {
        ComponentStorage<T>& source = *src.cast<T>();
        source.remove(row);
    };

    return ErasedComponentStorage{reinterpret_cast<void*>(componentStorage), deleter, cloner, copy, remove};
}

#else
ErasedComponentStorage::ErasedComponentStorage(ErasedComponentStorage&& rhs) {
    this->~ErasedComponentStorage();
    mDeleter = rhs.mDeleter;
    mCloner = rhs.mCloner;
    mRemover = rhs.mRemover;
    mCopier = rhs.mCopier;
    mRaw = rhs.mRaw;
    rhs.mRaw = nullptr;
}

ErasedComponentStorage& ErasedComponentStorage::operator=(ErasedComponentStorage&& rhs) {
    this->~ErasedComponentStorage();
    mDeleter = rhs.mDeleter;
    mCloner = rhs.mCloner;
    mRemover = rhs.mRemover;
    mCopier = rhs.mCopier;
    mRaw = rhs.mRaw;
    rhs.mRaw = nullptr;
    return *this;
}

ErasedComponentStorage::~ErasedComponentStorage() {
    if (mRaw) {
        mDeleter(mRaw);
    }
}

void ErasedComponentStorage::copyFrom(uint32_t srcRow, uint32_t dstRow, ErasedComponentStorage& src) {
    mCopier(*this, srcRow, dstRow, src);
}

void ErasedComponentStorage::remove(uint32_t rowIndex) { mRemover(*this, rowIndex); }

void ErasedComponentStorage::cloneTo(ErasedComponentStorage& retval) { mCloner(*this, retval); }
#endif