#include "entities.hpp"

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>

#include "javascript_bindings.hpp"
#endif

#include <set>

#ifdef ENTITIES_TEMPLATE_IMPL

template <typename... Args, typename... Rest>
Entities& Entities::addSystems(SystemSchedule schedule, System<Args...> system, Rest... rest) {
    static_assert((std::is_default_constructible_v<std::remove_reference_t<Args>> && ...));
    auto systemCaller = [this, system]() {
        if constexpr (sizeof...(Args) > 0) {
            auto args = std::make_tuple(std::remove_reference_t<Args>()...);
            (std::get<std::remove_reference_t<Args>>(args).fill(this), ...);
            system(std::get<std::remove_reference_t<Args>>(args)...);
        } else {
            system();
        }
    };

    switch (schedule) {
    case SystemSchedule::Startup: mStartupSystems.emplace_back(systemCaller); break;
    case SystemSchedule::Update: mUpdateSystems.emplace_back(systemCaller); break;
    case SystemSchedule::Shutdown: mShutdownSystems.emplace_back(systemCaller); break;
    }

    if constexpr (sizeof...(Rest) > 0) {
        addSystems(schedule, rest...);
    }
    return *this;
}

template <typename... T>
Query<T...> Entities::query() {
    Query<T...> result{};

    result.fill(this);
    return result;
}

template <typename... T>
void Entities::setGlobal(T... args) {
    (
        [&]() {
            auto index = std::type_index(typeid(T));
            if (auto iter = mGlobalVariables.find(index); iter != mGlobalVariables.end()) {
                *(reinterpret_cast<T*>(iter->second.get())) = std::move(args);
            } else {
                mGlobalVariables[std::type_index(typeid(T))] = std::make_shared<T>(std::move(args));
            }
        }(),
        ...);
}

template <typename... T>
Entities& Entities::addPlugin(Plugin plugin, T... rest) {
    mPlugins.push_back(std::move(plugin));

    if constexpr (sizeof...(T) > 0) {
        addPlugin(rest...);
    }
    return *this;
}

template <typename... T>
std::tuple<T&...> Entities::getGlobal() {
    auto hasGlobals = ((mGlobalVariables.find(std::type_index(typeid(T))) != mGlobalVariables.end()) && ...);

    assert(hasGlobals);

    return std::make_tuple<std::reference_wrapper<T>...>(
        *reinterpret_cast<T*>(mGlobalVariables[std::type_index(typeid(T))].get())...);
}

template <typename... T>
void Entities::setComponents(EntityID entityID, T... components) {
    static_assert(sizeof...(T) > 0, "There should be at least one component to be added");

    static_assert(UniqueTypes<T...>::value, "Component Types must be unique");

    auto& entityArchType = archtypeFromEntityID(entityID);

    // To be used to access entityArchType once mArchtypes has been mutated
    auto entityArchTypeIndex = mArchtypes.denseStorageIndex(entityArchType.getHash());

    auto rowIndex = entityRowFromID(entityID);

    if (entityArchType.hasComponents<T...>()) {
        (entityArchType.set<T>(rowIndex, std::move(components)), ...);
        // auto a = getComponents<T...>(entityID);
    } else {
        // This has to be done incase some of the component to be added already exists in the current archtype
        // of the entity
        auto uniqueComponents = std::set<std::type_index>{std::type_index(typeid(T))...};
        for (auto& type : entityArchType.componentTypes()) {
            uniqueComponents.emplace(type);
        }

        std::vector allComponentIndex(uniqueComponents.begin(), uniqueComponents.end());

        auto newHash = ArchTypeStorage::computeHash(allComponentIndex);

        ArchTypeStorage* newEntityArchtype = mArchtypes.get(newHash);

        if (newEntityArchtype == nullptr) {
            auto archtype = entityArchType.clone();

            // add component storage from the template
            (archtype.addComponent(std::type_index(typeid(T)), ErasedComponentStorage::create<T>()), ...);

            archtype.setHash(newHash);
            mArchtypes.put(newHash, std::move(archtype));
            newEntityArchtype = mArchtypes.get(newHash);
        }

        auto newRowIndex = newEntityArchtype->newRow(entityID);

        // The previous entityArchtype has to be accessed this way because the reference we had
        // earlier would already be invalidated by the mArchtypes.put call.
        auto& prevArchtype = mArchtypes.values()[entityArchTypeIndex];

        //  copy the entity row from the entities previouse archtype
        for (auto type : prevArchtype.componentTypes()) {
            newEntityArchtype->getComponent(type)->copyFrom(rowIndex, newRowIndex, *prevArchtype.getComponent(type));
        }
        // copy component values from the input to this method
        (newEntityArchtype->template set<T>(std::move(components)), ...);

        auto archTypeIndex = mArchtypes.denseStorageIndex(newHash);
        auto& ptr = mEntities[entityID];
        ptr.archtypeIndex = archTypeIndex;
        ptr.rowIndex = newRowIndex;

        // update the row index of the last entity in the previous archtypestorage if it was not the one that
        // was removed.
        if (auto lastEntityId = prevArchtype.entities().back(); lastEntityId != entityID) {
            mEntities[lastEntityId].rowIndex = rowIndex;
        }
        prevArchtype.removeRow(rowIndex);
    }
}

template <typename... T>
std::tuple<T&...> Entities::getComponents(EntityID entityID) {
    static_assert(sizeof...(T) > 0, "There should be at least one component to be retrieved");

    static_assert(UniqueTypes<T...>::value, "Component Types must be unique");

    auto& entityArchType = archtypeFromEntityID(entityID);
    auto rowIndex = entityRowFromID(entityID);

    auto hasComponents = entityArchType.hasComponents<T...>();
    assert(hasComponents);

    return std::make_tuple(std::reference_wrapper<T>(entityArchType.getRow<T>(rowIndex))...);
}

template <typename... T>
void Entities::removeComponents(EntityID entityID) {
    static_assert(sizeof...(T) > 0, "There should be at least one component to be removed");

    static_assert(UniqueTypes<T...>::value, "Component Types must be unique");

    auto& currentArchtype = archtypeFromEntityID(entityID);

    auto currentArchtypeIndex = mArchtypes.denseStorageIndex(currentArchtype.getHash());

    auto rowIndexInCurrentArchType = mEntities[entityID].rowIndex;

    bool hasComponents = currentArchtype.hasComponents<T...>();

    assert(hasComponents);

    std::set<std::type_index> componentsToRemove = {std::type_index(typeid(T))...};

    std::vector<std::type_index> componentsToRemainWithEntity;

    for (auto& type : currentArchtype.componentTypes()) {
        if (componentsToRemove.find(type) == componentsToRemove.end()) {
            componentsToRemainWithEntity.emplace_back(type);
        }
    }

    auto newHash = ArchTypeStorage::computeHash(componentsToRemainWithEntity);

    ArchTypeStorage* newArchtype = mArchtypes.get(newHash);

    if (newArchtype == nullptr) {
        auto archtype = currentArchtype.clone(componentsToRemainWithEntity);

        archtype.setHash(newHash);
        mArchtypes.put(newHash, std::move(archtype));
        newArchtype = mArchtypes.get(newHash);
    }

    auto& prevArchtype = mArchtypes.values()[currentArchtypeIndex];

    auto newRowIndex = newArchtype->newRow(entityID);

    for (auto type : componentsToRemainWithEntity) {
        newArchtype->getComponent(type)->copyFrom(rowIndexInCurrentArchType, newRowIndex,
                                                  *prevArchtype.getComponent(type));
    }

    auto archTypeIndex = mArchtypes.denseStorageIndex(newHash);
    auto& ptr = mEntities[entityID];
    ptr.archtypeIndex = archTypeIndex;
    ptr.rowIndex = newRowIndex;

    // update the row index of the last entity in the previous archtypestorage if it was not the one that
    // was removed.
    if (auto lastEntityId = prevArchtype.entities().back(); lastEntityId != entityID) {
        mEntities[lastEntityId].rowIndex = rowIndexInCurrentArchType;
    }
    prevArchtype.removeRow(rowIndexInCurrentArchType);
}

template <typename T>
void Entities::traverse(std::function<T(Entities&, EntityID, T)> callback, T startValue, EntityID rootEntityID) {
    auto& children = getChildren(rootEntityID);

    auto updatedValue =
        rootEntityID == Entities::ROOT_ENTITY_ID ? startValue : callback(*this, rootEntityID, startValue);

    for (auto child : children) {
        traverse(callback, updatedValue, child);
    }
}

template <typename... T>
bool Entities::hasComponents(EntityID entityID) {
    auto& archtype = archtypeFromEntityID(entityID);
    return archtype.hasComponents<T...>();
}

#else

Entities::Entities() {
    mArchtypes.put(Entities::VOID_ARCHTYPE_HASH, ArchTypeStorage{});

    setGlobal(AppState{.initialized = false, .initializationStage = "Creating World", .running = true});
};

EntityID Entities::newEntity(EntityID parentID) {
    mEntityCount++;

    EntityID newEntityID;

    if (mReusableEntityIDs.size() > 1) {
        newEntityID = *mReusableEntityIDs.begin();

        mReusableEntityIDs.erase(newEntityID);
    } else {
        newEntityID = mNextEntityId++;
    }

    auto voidArchType = mArchtypes.get(Entities::VOID_ARCHTYPE_HASH);
    auto entityRowIndex = voidArchType->newRow(newEntityID);
    mEntities[newEntityID] = {.archtypeIndex = 0, .rowIndex = entityRowIndex};

    mChildrenMap[parentID].push_back(newEntityID);

    mParentMap[newEntityID] = parentID;
    return newEntityID;
}

void Entities::update() {
    for (auto& system : mUpdateSystems) {
        system();
    }
}

void Entities::run() {
    auto [appState] = getGlobal<AppState>();

    appState.initializationStage = "Running plugins";

    // build plugins
    for (auto& plugin : mPlugins) {
        plugin(*this);
    }

    appState.initializationStage = "Running startup systems";

    for (auto& system : mStartupSystems) {
        system();
    }

    appState.initializationStage = "Initialized";
    appState.initialized = true;
#if defined(EMSCRIPTEN)
    emscripten_set_main_loop_arg(
        [](void* userData) {
            Entities& world = *reinterpret_cast<Entities*>(userData);
            world.update();
        },
        (void*)this, 0, true);
#else

    while (appState.running) {
        update();
    }

    for (auto& system : mShutdownSystems) {
        system();
    }

#endif
}

void Entities::removeEntity(EntityID entityID) {
    assert(entityID != Entities::ROOT_ENTITY_ID);

    _removeEntity(entityID, true);
}

void Entities::_removeEntity(EntityID entityID, bool removeFromParent) {
    auto childrenIter = mChildrenMap.find(entityID);

    if (childrenIter != mChildrenMap.end()) {
        for (auto child : childrenIter->second) {
            _removeEntity(child, false);
        }
    }

    auto& archtype = archtypeFromEntityID(entityID);

    auto& entityPtr = mEntities[entityID];

    if (auto lastEntityID = archtype.entities().back(); lastEntityID != entityID) {
        mEntities[lastEntityID].rowIndex = entityPtr.rowIndex;
    }

    archtype.removeRow(entityPtr.rowIndex);

    mEntityCount--;

    if (removeFromParent) {
        auto& parentChildren = mChildrenMap[mParentMap[entityID]];

        std::array<EntityID, 1> entityToSearch = {entityID};

        auto entityIterInParent =
            std::search(parentChildren.begin(), parentChildren.end(), entityToSearch.begin(), entityToSearch.end());

        parentChildren.erase(entityIterInParent);
    }
    mEntities.erase(entityID);
    mChildrenMap.erase(entityID);
    mParentMap.erase(entityID);

    mReusableEntityIDs.emplace(entityID);
}

EntityID Entities::getParent(EntityID entityID) {
    return mParentMap[entityID];
}

uint32_t Entities::entityCount() {
    return mEntityCount;
}

const std::vector<EntityID>& Entities::getChildren(EntityID entityID) {
    return mChildrenMap[entityID];
}

std::vector<ArchTypeStorage>& Entities::archtypes() {
    return mArchtypes.values();
}

ArchTypeStorage& Entities::archtypeFromEntityID(EntityID entityID) {
    auto entityIter = mEntities.find(entityID);

    assert(entityIter != mEntities.end());

    return mArchtypes.values()[entityIter->second.archtypeIndex];
}

uint32_t Entities::entityRowFromID(EntityID entityID) {
    auto entityIter = mEntities.find(entityID);

    assert(entityIter != mEntities.end());

    return entityIter->second.rowIndex;
}
#endif