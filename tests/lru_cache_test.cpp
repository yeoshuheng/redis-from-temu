#include "../storage/lru_cache.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

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
    cache.get(1);
    cache.add(3, "three");
    EXPECT_THROW(cache.get(2), std::runtime_error);
    EXPECT_EQ(cache.get(1), "one");
    EXPECT_EQ(cache.get(3), "three");
}

TEST(LRUCacheTest, UpdateExistingKeyResetsValueAndTTL) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one", 1000);
    cache.add(1, "ONE");
    EXPECT_EQ(cache.get(1), "ONE");
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
    cache.add(2, "two");
    EXPECT_THROW(cache.get(1), std::runtime_error);
    EXPECT_EQ(cache.get(2), "two");
}

TEST(LRUCacheTest, FrequentAccessPreventsEviction) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one");
    cache.add(2, "two");
    cache.get(1);
    cache.get(1);
    cache.add(3, "three");
    EXPECT_EQ(cache.get(1), "one");
    EXPECT_THROW(cache.get(2), std::runtime_error);
}

TEST(LRUCacheTest, TTLExpiration) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one", 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    EXPECT_THROW(cache.get(1), std::runtime_error);
}

TEST(LRUCacheTest, TTLDoesNotBreakLRU) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one", 50);
    cache.add(2, "two");
    cache.add(3, "three");
    EXPECT_THROW(cache.get(1), std::runtime_error);
    EXPECT_EQ(cache.get(2), "two");
    EXPECT_EQ(cache.get(3), "three");
}

TEST(LRUCacheTest, OverwriteClearsTTL) {
    LRUCache<int, std::string> cache(2);
    cache.add(1, "one", 50);
    cache.add(1, "ONE");
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    EXPECT_EQ(cache.get(1), "ONE");
}

TEST(LRUCacheTest, RemoveExpiredRemovesExpiredKeys) {
    LRUCache<int, std::string> cache(3);
    cache.add(1, "one", 50);
    cache.add(2, "two", 50);
    cache.add(3, "three");
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    cache.remove_expired();
    EXPECT_THROW(cache.get(1), std::runtime_error);
    EXPECT_THROW(cache.get(2), std::runtime_error);
    EXPECT_EQ(cache.get(3), "three");
}