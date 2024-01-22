#include "archtype.hpp"

#ifdef ARCHTYPE_TEMPLATE_IMPL
template <typename T>
void ArchTypeStorage::set(uint32_t rowIndex, T component) {
    assert(mEntityIDs.size() > rowIndex);

    auto index = std::type_index(typeid(T));
    auto& storage = *mComponents.get(index);
    ComponentStorage<T>& typedStorage = *storage.cast<T>();
    typedStorage[rowIndex] = std::move(component);
}

template <typename T>
void ArchTypeStorage::set(T component) {
    auto index = std::type_index(typeid(T));
    auto& storage = *mComponents.get(index);
    ComponentStorage<T>& typedStorage = *storage.cast<T>();
    auto row = typedStorage.put(std::move(component));
    assert(row == mEntityIDs.size() - 1);
}

template <typename... T>
bool ArchTypeStorage::hasComponents() {
    return (hasComponent<T>() && ...);
}

template <typename... T>
ArchTypeStorage ArchTypeStorage::create() {
    auto archType = ArchTypeStorage();

    archType.mHash = computeHash<T...>();

    size_t size = sizeof...(T);

    archType.mComponents.reserve(size);
    archType.mComponentTypes.reserve(size);

    (
        [&]() {
            auto index = std::type_index(typeid(T));
            archType.mComponentTypes.emplace_back(index);
            archType.mComponents.put(index, ErasedComponentStorage::create<T>());
        }(),
        ...);
    return archType;
}

template <typename T>
bool ArchTypeStorage::hasComponent() {
    auto key = std::type_index(typeid(T));
    return hasComponent(key);
}

template <typename... T>
u_int64_t ArchTypeStorage::computeHash() {
    std::vector<std::type_index> types{std::type_index(typeid(T))...};
    return computeHash(types);
}

template <typename T>
T& ArchTypeStorage::getRow(uint32_t rowIndex) {
    auto index = std::type_index(typeid(T));
    return (*getComponent(index)->cast<T>())[rowIndex];
}

#else

uint32_t ArchTypeStorage::newRow(EntityID entity) {
    auto newRowIndex = mEntityIDs.size();
    mEntityIDs.emplace_back(entity);
    return newRowIndex;
}

bool ArchTypeStorage::hasComponent(std::type_index type) {
    if (mComponents.get(type)) {
        return true;
    }
    return false;
}

u_int64_t ArchTypeStorage::computeHash(std::vector<std::type_index>& types) {
    std::sort(types.begin(), types.end(), std::greater<std::type_index>());

    u_int64_t hash = 0;

    for (auto& type : types) {
        hash ^= type.hash_code();
    }
    return hash;
}

void ArchTypeStorage::computeHash() { mHash = computeHash(mComponentTypes); }

void ArchTypeStorage::setHash(u_int64_t hash) { mHash = hash; }

u_int64_t ArchTypeStorage::getHash() { return mHash; }

std::vector<EntityID>& ArchTypeStorage::entities() { return mEntityIDs; }

void ArchTypeStorage::removeRow(uint32_t rowIndex) {
    assert(mEntityIDs.size() > rowIndex);

    std::swap(mEntityIDs[rowIndex], mEntityIDs.back());
    mEntityIDs.pop_back();
    for (auto& component : mComponents.values()) {
        component.remove(rowIndex);
    }
}

std::vector<std::type_index>& ArchTypeStorage::componentTypes() { return mComponentTypes; }

ArchTypeStorage ArchTypeStorage::fromTypeIndex(std::vector<std::type_index>& types) {
    ArchTypeStorage archType;

    for (auto& type : types) {
        archType.addComponent(type);
    }

    return archType;
}

ArchTypeStorage ArchTypeStorage::clone() {
    auto storageClone = fromTypeIndex(mComponentTypes);

    for (auto& type : mComponentTypes) {
        getComponent(type)->cloneTo(*storageClone.getComponent(type));
    }

    return storageClone;
}

ArchTypeStorage ArchTypeStorage::clone(std::vector<std::type_index>& types) {
    for (auto& type : types) {
        assert(hasComponent(type));
    }

    auto storageClone = fromTypeIndex(types);

    for (auto& type : types) {
        getComponent(type)->cloneTo(*storageClone.getComponent(type));
    }

    return storageClone;
}

ArrayHashMap<std::type_index, ErasedComponentStorage>& ArchTypeStorage::components() { return mComponents; }

void ArchTypeStorage::addComponent(std::type_index type) {
    if (hasComponent(type)) {
        return;
    }
    mComponentTypes.emplace_back(type);
    mComponents.put(type, ErasedComponentStorage());
}

void ArchTypeStorage::addComponent(std::type_index type, ErasedComponentStorage componenetStorage) {
    if (hasComponent(type)) {
        return;
    }
    mComponentTypes.emplace_back(type);
    mComponents.put(type, std::move(componenetStorage));
}

ErasedComponentStorage* ArchTypeStorage::getComponent(std::type_index& type) { return mComponents.get(type); }
#endif