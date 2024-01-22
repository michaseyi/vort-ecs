#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

#include "component_storage.hpp"

class ErasedComponentStorage {
public:
    using Deleter = std::function<void(void*)>;
    using Cloner = std::function<void(ErasedComponentStorage&, ErasedComponentStorage&)>;
    using Copier = std::function<void(ErasedComponentStorage&, uint32_t, uint32_t, ErasedComponentStorage&)>;
    using Remover = std::function<void(ErasedComponentStorage&, uint32_t)>;

    ErasedComponentStorage() = default;

    ErasedComponentStorage(void* rawPtr, Deleter deleter, Cloner cloner, Copier copier, Remover remover)
        : mRaw(rawPtr), mDeleter(deleter), mCloner(cloner), mCopier(copier), mRemover(remover) {
    }

    ErasedComponentStorage(ErasedComponentStorage&& rhs);

    ErasedComponentStorage& operator=(ErasedComponentStorage&& rhs);

    ~ErasedComponentStorage();

    template <typename T>
    ComponentStorage<T>* cast();

    template <typename T>
    static ErasedComponentStorage create();

    void copyFrom(uint32_t srcRow, uint32_t dstRow, ErasedComponentStorage& src);

    void remove(uint32_t rowIndex);

    void cloneTo(ErasedComponentStorage& retval);

private:
    void* mRaw = nullptr;
    Deleter mDeleter;
    Cloner mCloner;
    Copier mCopier;
    Remover mRemover;
};

#define ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL
#include "erased_component.cpp"
#undef ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL