#include "ecs.hpp"
#include <memory>

struct Position {
    float x = 0;
    float y = 0;
    float z = 0;
};

struct Scale {
    float x = 0;
    float y = 0;
    float z = 0;
};

struct Buffer {
    std::unique_ptr<int> data;
    Buffer() : data(new int()) {}
};

struct Animation {};
int main() {
    const int ENTITY_COUNT = 20000;

    auto world = Entities();

    std::vector<EntityID> players;

    players.reserve(ENTITY_COUNT);

    for (int i{0}; i < ENTITY_COUNT; i++) {
        auto player = world.newEntity();
        world.setComponents<Position, Scale, Buffer>(player, {}, {}, {});
        players.emplace_back(player);
    }

    for (auto player : players) {
        world.setComponents<Animation>(player, {});
    }

    auto [scale] = world.getComponents<Scale>(players[10]);
    std::cout << scale.x << std::endl;
}
