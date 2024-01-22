#pragma once
#include <cstdint>

#include "entities.hpp"
template <typename... T>
class Tags {};

template <typename... T>
class Columns {};

template <typename... T>
class Query {
    static_assert(false);
};

template <typename... T, typename... U>
class Query<Columns<T...>, Tags<U...>> {
    static_assert(UniqueTypes<T...>::value);
    static_assert(UniqueTypes<U...>::value);

public:
    static inline std::vector<std::type_index> queryComponents = {std::type_index(typeid(T))...,
                                                                  std::type_index(typeid(U))...};

    void fill(Entities *world) {
        mWorld = world;

        auto &archtypes = world->archtypes();

        for (uint32_t i = 1; i < archtypes.size(); i++) {
            if (archtypes[i].hasComponents<T..., U...>() && archtypes[i].entities().size() > 0) {
                mStartingArchtypeIndex = i;
                mStartingRowIndex = 0;
                mEmptyResult = false;
                break;
            }
        }
    };

    // It is important to note that operations that could create or remove archtypes, create or remove component
    // storages, affect the row index of an entity should not be performed while a query object iterator is in use
    // as it can lead to undefined behaviour. Only after you are done using the iterator is it safe to perform these
    // actions.

    // TODO: invalidate any iterator when any of the cases above happens
    class Iterator {
    public:
        Iterator(Entities *world, uint32_t currentArchtypeIndex, uint32_t currentRowIndex, bool ended = false)
            : mWorld(world),
              mCurrentArchtypeIndex(currentArchtypeIndex),
              mCurrentRowIndexInArchtype(currentRowIndex),
              mEnded(ended) {
            std::vector<std::type_index> queryComponents = {std::type_index(typeid(T))...,
                                                            std::type_index(typeid(U))...};
        }

        Iterator &operator++(int) {
            return next();
        }

        Iterator &operator++() {
            return next();
        }

        inline Iterator &next() {
            assert(!mEnded);

            auto &archtypes = mWorld->archtypes();
            auto &currentArchtype = archtypes[mCurrentArchtypeIndex];

            if (currentArchtype.entities().size() > mCurrentRowIndexInArchtype + 1) {
                mCurrentRowIndexInArchtype++;
            } else {
                for (size_t i = mCurrentArchtypeIndex + 1; i < archtypes.size(); i++) {
                    if (archtypes[i].hasComponents<T..., U...>()) {
                        mCurrentArchtypeIndex = i;
                        mCurrentRowIndexInArchtype = 0;
                        return *this;
                    }
                }
                mEnded = true;
                mCurrentArchtypeIndex = 0;
                mCurrentRowIndexInArchtype = 0;
            }
            return *this;
        }

        std::tuple<T &...> operator*() {
            assert(!mEnded);

            // TODO: optimize so we use the linear array index to access component storages instead of going through the
            // hash map of the type everytime. this would be very useful when the next entity with the required types is
            // in the current archtype.

            auto &archtype = mWorld->archtypes()[mCurrentArchtypeIndex];
            return std::make_tuple(std::reference_wrapper<T>(archtype.getRow<T>(mCurrentRowIndexInArchtype))...);
        }

        bool operator!=(const Iterator &rhs) {
            return !(mEnded == rhs.mEnded && mCurrentArchtypeIndex == rhs.mCurrentArchtypeIndex &&
                     mCurrentRowIndexInArchtype == rhs.mCurrentRowIndexInArchtype);
        };

    private:
        Entities *mWorld;
        uint32_t mCurrentArchtypeIndex;
        uint32_t mCurrentRowIndexInArchtype;
        bool mEnded = false;
    };

    Iterator begin() {
        return Iterator{mWorld, mStartingArchtypeIndex, mStartingRowIndex, mEmptyResult};
    };
    Iterator end() {
        return Iterator{mWorld, 0, 0, true};
    };

private:
    Entities *mWorld;
    uint32_t mStartingArchtypeIndex = 0;
    uint32_t mStartingRowIndex = 0;
    bool mEmptyResult = true;
};
