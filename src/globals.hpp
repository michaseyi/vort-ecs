#pragma once
#include <cstdint>
#include <iostream>
template <typename... T>
class Global {
public:
    Global(){};
    void fill(Entities *world) {
        mWorld = world;
        auto globals = mWorld->getGlobal<T...>();

        (
            [&]() {
                std::get<T *>(mValues) = &std::get<T &>(globals);
            }(),
            ...);
    }

    const std::tuple<T *...> &values() const {
        return mValues;
    }

private:
    std::tuple<T *...> mValues;
    Entities *mWorld;
};

template <typename... T>
struct std::tuple_size<Global<T...>> : std::integral_constant<size_t, sizeof...(T)> {};

template <size_t I, typename... T>
struct std::tuple_element<I, Global<T...>> {
    using type = decltype(std::get<I>(std::tuple<T...>())) &;
};

template <size_t I, typename... T>
auto &get(const Global<T...> &obj) {
    return *std::get<I>(obj.values());
}
