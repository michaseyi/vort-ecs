#pragma once
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <type_traits>

#include "archtype.hpp"
#include "array_hashmap.hpp"

template <typename... T>
class Query;

struct AppState {
    bool initialized;
    std::string initializationStage;
    bool running;
};

// TODO: Temporary Entity ID solution. Replace with UUIDs;
using EntityID = uint32_t;

enum class SystemSchedule {
    Startup,
    Update,
    Shutdown,
};

template <typename...>
struct UniqueTypes : std::true_type {};

template <typename T, typename... Rest>
struct UniqueTypes<T, Rest...>
    : std::integral_constant<bool, (!std::is_same_v<T, Rest> && ...) && UniqueTypes<Rest...>::value> {};

class Entities {
public:
    template <typename... Args>
    using System = void (*)(Args...);

    using Plugin = std::function<void(Entities&)>;

    const static inline uint64_t VOID_ARCHTYPE_HASH = std::numeric_limits<uint64_t>::max();

    const static inline uint32_t ROOT_ENTITY_ID = 0;

    Entities(const Entities&) = delete;

    EntityID getParent(EntityID entityID);

    const std::vector<EntityID>& getChildren(EntityID parentID = Entities::ROOT_ENTITY_ID);

    EntityID newEntity(EntityID parentID = Entities::ROOT_ENTITY_ID);

    void removeEntity(EntityID entityID);

    template <typename... T>
    void setComponents(EntityID entityID, T... components);

    template <typename... T>
    std::tuple<T&...> getComponents(EntityID entityID);

    template <typename... T>
    void removeComponents(EntityID entityID);

    struct Pointer {
        u_int16_t archtypeIndex;
        uint32_t rowIndex;
    };

    std::vector<ArchTypeStorage>& archtypes();

    void run();

    template <typename... T>
    Entities& addPlugin(Plugin plugin, T... rest);

    template <typename... Args, typename... Rest>
    Entities& addSystems(SystemSchedule schedule, System<Args...> system, Rest... rest);

    template <typename... T>
    void setGlobal(T... args);

    template <typename... T>
    std::tuple<T&...> getGlobal();

    template <typename... T>
    Query<T...> query();

    void update();

    uint32_t entityCount();

    Entities();

    template <typename... T>
    bool hasComponents(EntityID entityID);

    template <typename T>
    void traverse(std::function<T(Entities&, EntityID, T)>, T, EntityID rootEntityID = Entities::ROOT_ENTITY_ID);

private:
    void _removeEntity(EntityID entityID, bool removeFromParent);

    uint32_t mEntityCount = 0;
    uint32_t mNextEntityId = 1;

    std::unordered_map<EntityID, Pointer> mEntities;
    ArrayHashMap<uint64_t, ArchTypeStorage> mArchtypes;

    ArchTypeStorage& archtypeFromEntityID(EntityID entityID);

    uint32_t entityRowFromID(EntityID entityID);

    std::unordered_map<std::type_index, std::shared_ptr<void>> mGlobalVariables;

    std::vector<std::function<void(void)>> mStartupSystems;
    std::vector<std::function<void(void)>> mUpdateSystems;
    std::vector<std::function<void(void)>> mShutdownSystems;

    std::vector<Plugin> mPlugins;

    std::map<EntityID, std::vector<EntityID>> mChildrenMap;

    std::map<EntityID, EntityID> mParentMap;

    std::unordered_set<EntityID> mReusableEntityIDs;
};

#define ENTITIES_TEMPLATE_IMPL
#include "entities.cpp"
#undef ENTITIES_TEMPLATE_IMPL