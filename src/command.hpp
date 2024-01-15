#pragma once
#include "entities.hpp"

class Command {
public:
    void fill(Entities* world) { mWorld = world; }

    template <typename... T>
    EntityID spawn(T... args) {
        assert(mWorld);

        auto entity = mWorld->newEntity();
        mWorld->setComponents<T...>(entity, std::move(args)...);
        return entity;
    }

    template <typename... T>
    EntityID spawn(EntityID parent, T... args) {
        assert(mWorld);

        auto entity = mWorld->newEntity(parent);
        mWorld->setComponents(entity, std::move(args)...);
        return entity;
    }

    template <typename... T>
    void removeComponents(EntityID entity) {
        assert(mWorld);

        mWorld->removeComponents<T...>(entity);
    }

    template <typename... T>
    void setGlobal(T... values) {
        assert(mWorld);
        mWorld->setGlobal<T...>(std::move(values)...);
    }

    template <typename... T>
    auto getGlobal() {
        return mWorld->getGlobal<T...>();
    }

private:
    Entities* mWorld;
};