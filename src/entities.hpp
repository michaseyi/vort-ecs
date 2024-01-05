#pragma once
#include "archtype.hpp"
#include "array_hashmap.hpp"
#include "limits"

#include <cassert>
#include <iostream>
#include <type_traits>

using EntityID = u_int32_t;

template <typename...> struct UniqueTypes : std::true_type {};

template <typename T, typename... Rest>
struct UniqueTypes<T, Rest...>
    : std::integral_constant<bool, (!std::is_same_v<T, Rest> && ...) && UniqueTypes<Rest...>::value> {};

class Entities {
public:
    Entities();

    EntityID newEntity();

    EntityID newEntity(EntityID parentID);

    template <typename... T> void setComponents(EntityID entityID, T&&... components);

    template <typename... T> std::tuple<T...> getComponents(EntityID entityID);

    template <typename... T> void removeComponents(EntityID entityID);

    struct Pointer {
        u_int16_t archtypeIndex;
        u_int32_t rowIndex;
    };

private:
    u_int32_t mEntityCount = 0;
    std::unordered_map<EntityID, Pointer> mEntities;
    ArrayHashMap<u_int64_t, ArchTypeStorage> mArchtypes;

    const static inline u_int64_t voidArchtypeHash = std::numeric_limits<u_int64_t>::max();

    ArchTypeStorage& archtypeFromEntityID(EntityID entityID);

    u_int32_t entityRowFromID(EntityID entityID);
};

#define ENTITIES_TEMPLATE_IMPL
#include "entities.cpp"
#undef ENTITIES_TEMPLATE_IMPL