//
// Created by Yeo Shu Heng on 17/4/26.
//

#include "../../include/core/lru_cache.hpp"

#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using core::LRUCache;

TEST(LRUCacheTest, InsertAndGet) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one");
    cache.add(2, "two");

    EXPECT_EQ(cache.get(1).value(), "one");
    EXPECT_EQ(cache.get(2).value(), "two");
}

TEST(LRUCacheTest, EvictsLeastRecentlyUsed) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one");
    cache.add(2, "two");

    cache.get(1);
    cache.add(3, "three");

    EXPECT_FALSE(cache.get(2).has_value());
    EXPECT_EQ(cache.get(1).value(), "one");
    EXPECT_EQ(cache.get(3).value(), "three");
}

TEST(LRUCacheTest, UpdateExistingKeyResetsValueAndTTL) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one", 1000);
    cache.add(1, "ONE");

    EXPECT_EQ(cache.get(1).value(), "ONE");
}

TEST(LRUCacheTest, RemoveKey) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one");
    cache.add(2, "two");

    EXPECT_TRUE(cache.remove(1));

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_EQ(cache.get(2).value(), "two");
}

TEST(LRUCacheTest, CapacityOne) {
    LRUCache<int, std::string> cache(1);
    cache.add(1, "one");

    EXPECT_EQ(cache.get(1).value(), "one");

    cache.add(2, "two");

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_EQ(cache.get(2).value(), "two");
}

TEST(LRUCacheTest, FrequentAccessPreventsEviction) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one");
    cache.add(2, "two");

    cache.get(1);
    cache.get(1);

    cache.add(3, "three");

    EXPECT_EQ(cache.get(1).value(), "one");
    EXPECT_FALSE(cache.get(2).has_value());
}

TEST(LRUCacheTest, TTLExpiration) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one", 50);

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    EXPECT_FALSE(cache.get(1).has_value());
}

TEST(LRUCacheTest, TTLDoesNotBreakLRU) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one", 50);
    cache.add(2, "two");
    cache.add(3, "three");

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_EQ(cache.get(2).value(), "two");
    EXPECT_EQ(cache.get(3).value(), "three");
}

TEST(LRUCacheTest, OverwriteClearsTTL) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one", 50);
    cache.add(1, "ONE");

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    EXPECT_EQ(cache.get(1).value(), "ONE");
}

TEST(LRUCacheTest, RemoveExpiredRemovesExpiredKeys) {
    LRUCache<int, std::string> cache(3);
    cache.add(1, "one", 50);
    cache.add(2, "two", 50);
    cache.add(3, "three");

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    cache.remove_expired();

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_FALSE(cache.get(2).has_value());
    EXPECT_EQ(cache.get(3).value(), "three");
}

TEST(LRUCacheTest, RemoveExpiredZeroBudgetMeansUnlimited) {
    LRUCache<int, std::string> cache(10);

    cache.add(1, "one", 50);
    cache.add(2, "two", 50);

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    cache.remove_expired(0);

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_FALSE(cache.get(2).has_value());
}

TEST(LRUCacheTest, RemoveExpiredRespectsBudget) {
    LRUCache<int, std::string> cache(10);

    cache.add(1, "one", 50);
    cache.add(2, "two", 50);
    cache.add(3, "three", 50);
    cache.add(4, "four", 50);

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    cache.remove_expired(2);

    int missing = 0;

    auto check = [&](int key) {
        if (!cache.get(key).has_value()) {
            missing++;
        }
    };

    check(1);
    check(2);
    check(3);
    check(4);

    EXPECT_EQ(missing, 4);
}