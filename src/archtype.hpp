#pragma once
#include <cassert>
#include <cstdint>
#include <iostream>
#include <typeindex>
#include <typeinfo>
#include <unordered_set>
#include <vector>

#include "array_hashmap.hpp"
#include "erased_component.hpp"

using EntityID = uint32_t;
class ArchTypeStorage {
public:
    uint32_t newRow(EntityID entity);

    template <typename T>
    bool hasComponent();

    bool hasComponent(std::type_index type);

    template <typename... T>
    static u_int64_t computeHash();

    static u_int64_t computeHash(std::vector<std::type_index>& types);

    template <typename... T>
    static ArchTypeStorage create();

    void computeHash();

    void setHash(u_int64_t hash);

    u_int64_t getHash();

    std::vector<EntityID>& entities();

    void removeRow(uint32_t rowIndex);

    std::vector<std::type_index>& componentTypes();

    static ArchTypeStorage fromTypeIndex(std::vector<std::type_index>& types);

    ArchTypeStorage clone();

    ArchTypeStorage clone(std::vector<std::type_index>& type);

    template <typename T>
    void set(uint32_t rowIndex, T component);

    template <typename T>
    void set(T component);

    ArrayHashMap<std::type_index, ErasedComponentStorage>& components();

    template <typename... T>
    bool hasComponents();

    void addComponent(std::type_index type);

    void addComponent(std::type_index type, ErasedComponentStorage componenetStorage);

    ErasedComponentStorage* getComponent(std::type_index& type);

    template <typename T>
    T& getRow(uint32_t rowIndex);

private:
    u_int64_t mHash = 0;
    std::vector<EntityID> mEntityIDs;
    std::vector<std::type_index> mComponentTypes;
    ArrayHashMap<std::type_index, ErasedComponentStorage> mComponents;

    // TODO: Use a set for tag components since they require no data. Treating them as regular components would waste 1
    // byte per entity that is assigned the tag.
    std::unordered_set<std::type_index> mTagComponents;
};

#define ARCHTYPE_TEMPLATE_IMPL
#include "archtype.cpp"
#undef ARCHTYPE_TEMPLATE_IMPL