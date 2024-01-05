#pragma once

#include "component_storage.hpp"
#include <functional>
#include <iostream>
#include <vector>

class ErasedComponentStorage {
public:
    using Deleter = std::function<void(void*)>;
    using Cloner = std::function<void(ErasedComponentStorage&, ErasedComponentStorage&)>;
    using Copier = std::function<void(ErasedComponentStorage&, u_int32_t, u_int32_t, ErasedComponentStorage&)>;
    using Remover = std::function<void(ErasedComponentStorage&, u_int32_t)>;

    ErasedComponentStorage() = default;

    ErasedComponentStorage(void* rawPtr, Deleter deleter, Cloner cloner, Copier copier, Remover remover)
        : mRaw(rawPtr), deleter(deleter), cloner(cloner), copier(copier), remover(remover) {}

    ErasedComponentStorage(ErasedComponentStorage&& rhs);

    ErasedComponentStorage& operator=(ErasedComponentStorage&& rhs);

    ~ErasedComponentStorage();

    template <typename T> ComponentStorage<T>* cast();

    template <typename T> static ErasedComponentStorage create();

    void copyFrom(u_int32_t srcRow, u_int32_t dstRow, ErasedComponentStorage& src);

    void remove(u_int32_t rowIndex);

    void cloneTo(ErasedComponentStorage& retval);

private:
    Deleter deleter;
    Cloner cloner;
    Copier copier;
    Remover remover;
    void* mRaw = nullptr;
};

#define ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL
#include "erased_component.cpp"
#undef ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL