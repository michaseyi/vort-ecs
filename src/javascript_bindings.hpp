#pragma once
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

// events bindings

// entity component update bindings

using EntityID = uint32_t;

template <typename T>
void dispatch_component_update(EntityID entityID, T& updatedValue) {
    std::type_index index(typeid(T));
    std::cout << index.name() << "Component does not have a dispatch_component_update partial template" << std::endl;
}
