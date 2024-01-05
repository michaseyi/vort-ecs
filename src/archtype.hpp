#pragma once
#include "array_hashmap.hpp"
#include "erased_component.hpp"

#include <cassert>
#include <iostream>
#include <typeindex>
#include <typeinfo>
#include <vector>

using EntityID = u_int32_t;

class ArchTypeStorage {
public:
    u_int32_t newRow(EntityID entity);

    template <typename T> bool hasComponent();

    bool hasComponent(std::type_index type);

    template <typename... T> static u_int64_t computeHash();

    static u_int64_t computeHash(std::vector<std::type_index>& types);

    template <typename... T> static ArchTypeStorage create();

    void computeHash();

    void setHash(u_int64_t hash);

    u_int64_t getHash();

    std::vector<EntityID>& entities();

    void removeRow(u_int32_t rowIndex);

    std::vector<std::type_index>& componentTypes();

    static ArchTypeStorage fromTypeIndex(std::vector<std::type_index>& types);

    ArchTypeStorage clone();

    ArchTypeStorage clone(std::vector<std::type_index>& type);

    template <typename T> void set(u_int32_t rowIndex, T& component);

    template <typename T> void set(T& component);

    ArrayHashMap<std::type_index, ErasedComponentStorage>& components();

    template <typename... T> bool hasComponents();

    void addComponent(std::type_index type);

    void addComponent(std::type_index type, ErasedComponentStorage componenetStorage);

    ErasedComponentStorage* getComponent(std::type_index& type);

private:
    u_int64_t mHash = 0;
    std::vector<EntityID> mEntityIDs;
    std::vector<std::type_index> mComponentTypes;
    ArrayHashMap<std::type_index, ErasedComponentStorage> mComponents;
};

#define ARCHTYPE_TEMPLATE_IMPL
#include "archtype.cpp"
#undef ARCHTYPE_TEMPLATE_IMPL