#pragma once
#include <cstdint>
#include <exception>
#include <iostream>
#include <unordered_map>
#include <vector>
template <typename K, typename V>
class ArrayHashMap {
public:
    template <typename _K, typename _V>
    uint32_t put(_K&& key, _V value) {
        return _put(std::forward<_K&>(key), std::move(value));
    }

    void reserve(size_t size) {
        mStorage.reserve(size);
        mIndexMap.reserve(size);
    }

    // Note the data returned is valid as long as the underlyning datastructure has not been resized.
    V* get(const K& key) {
        if (auto storageIndexIter = mIndexMap.find(key); storageIndexIter != mIndexMap.end()) {
            return &mStorage[storageIndexIter->second];
        }
        return nullptr;
    }

    std::vector<V>& values() {
        return mStorage;
    }

    inline uint32_t denseStorageIndex(K key) {
        return mIndexMap[key];
    }

private:
    uint32_t _put(const K& key, V value) {
        if (auto indexIter = mIndexMap.find(key); indexIter != mIndexMap.end()) {
            mStorage[indexIter->second] = std::move(value);
            return indexIter->second;
        } else {
            size_t indexForNewValue = mStorage.size();
            mIndexMap[key] = indexForNewValue;
            mStorage.emplace_back(std::move(value));
            return indexForNewValue;
        }
    }

    std::vector<V> mStorage;
    std::unordered_map<K, uint32_t> mIndexMap;
};