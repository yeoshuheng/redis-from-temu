//
// Created by Yeo Shu Heng on 14/4/26.
//

#include "../storage/lru_cache.hpp"

#include <gtest/gtest.h>

using storage::LRUCache;

TEST(LRUCacheTest, InsertAndGet) {
    LRUCache<int, std::string> cache(2);

    cache.add(1, "one");
    cache.add(2, "two");

    EXPECT_EQ(cache.get(1), "one");
    EXPECT_EQ(cache.get(2), "two");
}

TEST(LRUCacheTest, EvictsLeastRecentlyUsed) {
    LRUCache<int, std::string> cache(2);

    cache.add(1, "one");
    cache.add(2, "two");

    // access 1 → makes 2 LRU
    cache.get(1);

    cache.add(3, "three");  // should evict key 2

    EXPECT_THROW(cache.get(2), std::runtime_error);
    EXPECT_EQ(cache.get(1), "one");
    EXPECT_EQ(cache.get(3), "three");
}

TEST(LRUCacheTest, UpdateExistingKey) {
    LRUCache<int, std::string> cache(2);

    cache.add(1, "one");
    cache.add(2, "two");

    cache.add(1, "ONE");  // update

    EXPECT_EQ(cache.get(1), "ONE");

    cache.add(3, "three");  // should evict key 2

    EXPECT_THROW(cache.get(2), std::runtime_error);
}

TEST(LRUCacheTest, RemoveKey) {
    LRUCache<int, std::string> cache(2);

    cache.add(1, "one");
    cache.add(2, "two");

    cache.remove(1);

    EXPECT_THROW(cache.get(1), std::runtime_error);
    EXPECT_EQ(cache.get(2), "two");
}

TEST(LRUCacheTest, CapacityOne) {
    LRUCache<int, std::string> cache(1);

    cache.add(1, "one");
    EXPECT_EQ(cache.get(1), "one");

    cache.add(2, "two");  // should evict 1

    EXPECT_THROW(cache.get(1), std::runtime_error);
    EXPECT_EQ(cache.get(2), "two");
}

TEST(LRUCacheTest, FrequentAccessPreventsEviction) {
    LRUCache<int, std::string> cache(2);

    cache.add(1, "one");
    cache.add(2, "two");

    // Keep touching 1
    cache.get(1);
    cache.get(1);

    cache.add(3, "three");  // should evict 2

    EXPECT_EQ(cache.get(1), "one");
    EXPECT_THROW(cache.get(2), std::runtime_error);
}