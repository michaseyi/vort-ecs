#include <memory.h>

#include <thread>

#include "ecs.hpp"

using World = Entities;

struct Position {
    float x = 0;
    float y = 0;
    float z = 0;
};

struct Scale {
    float x = 1;
    float y = 1;
    float z = 1;
};

struct Orientation {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
};

EntityID createMesh(Entities& world, EntityID parent = Entities::ROOT_ENTITY_ID) {
    auto newEntityID = world.newEntity(parent);

    world.setComponents(newEntityID, Position{}, Scale{}, Orientation{});
    return newEntityID;
}

int main() {
    auto world = World();

    auto entity1 = createMesh(world);
    auto entity2 = createMesh(world, entity1);
    auto entity3 = createMesh(world, entity1);

    auto entity4 = createMesh(world, entity2);
    auto entity5 = createMesh(world, entity2);
    auto entity6 = createMesh(world, entity2);

    auto entity7 = createMesh(world, entity3);
    auto entity8 = createMesh(world, entity3);

    auto entity9 = createMesh(world, entity6);
    auto entity10 = createMesh(world, entity6);
    auto entity11 = createMesh(world, entity6);

    auto entity12 = createMesh(world, entity9);

    world.removeEntity(entity1);

    return 0;
}