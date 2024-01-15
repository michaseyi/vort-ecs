#pragma once
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <type_traits>

#include "archtype.hpp"
#include "array_hashmap.hpp"
#include "limits"

class Timer {
public:
    using clock = std::chrono::high_resolution_clock;
    Timer() : mStart(clock::now()) {}

    template <typename T>
    T elapsed() {
        return std::chrono::duration_cast<T>(clock::now() - mStart);
    }

private:
    std::chrono::time_point<clock> mStart;
};

struct Running {
    bool value = true;
};

using EntityID = u_int32_t;
template <typename... Args>
using System = void (*)(Args...);

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
    Entities(const Entities&) = delete;

    static inline Entities* instancePtr = nullptr;

    static Entities& getInstance() {
        if (!instancePtr) {
            instancePtr = new Entities();
        }
        return *instancePtr;
    };

    static void deleteInstance() {
        if (instancePtr) {
            delete instancePtr;
        }
    }

    EntityID newEntity();

    EntityID newEntity(EntityID parentID);

    void removeEntity(EntityID entityID);

    template <typename... T>
    void setComponents(EntityID entityID, T... components);

    template <typename... T>
    std::tuple<T&...> getComponents(EntityID entityID);

    template <typename... T>
    void removeComponents(EntityID entityID);

    struct Pointer {
        u_int16_t archtypeIndex;
        u_int32_t rowIndex;
    };

    std::vector<ArchTypeStorage>& archtypes();

    void run();

    template <typename... T>
    Entities& addSystems(SystemSchedule schedule);

    template <typename... Args, typename... Rest>
    Entities& addSystems(SystemSchedule schedule, System<Args...> system, Rest... rest);

    template <typename... T>
    void setGlobal(T... args);

    template <typename... T>
    std::tuple<T&...> getGlobal();

    void update();

private:
    Entities();

    u_int32_t mEntityCount = 0;
    std::unordered_map<EntityID, Pointer> mEntities;
    ArrayHashMap<u_int64_t, ArchTypeStorage> mArchtypes;

    const static inline u_int64_t voidArchtypeHash = std::numeric_limits<u_int64_t>::max();

    ArchTypeStorage& archtypeFromEntityID(EntityID entityID);

    u_int32_t entityRowFromID(EntityID entityID);

    std::unordered_map<std::type_index, std::shared_ptr<void>> mGlobalVariables;

    std::vector<std::function<void(void)>> mStartupSystems;
    std::vector<std::function<void(void)>> mUpdateSystems;
    std::vector<std::function<void(void)>> mShutdownSystems;
};

#define ENTITIES_TEMPLATE_IMPL
#include "entities.cpp"
#undef ENTITIES_TEMPLATE_IMPL