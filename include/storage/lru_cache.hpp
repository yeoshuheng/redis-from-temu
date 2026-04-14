//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <chrono>
#include <memory_resource>
#include <unordered_map>

namespace storage {
using Clock = std::chrono::steady_clock;
template <typename K, typename V>
struct LRUNode {
    using n_ptr = LRUNode<K, V>*;

    K key;
    V value;
    n_ptr prev;
    n_ptr next;
    std::optional<Clock::time_point> expired_t;

    LRUNode() : key(), value(), prev(nullptr), next(nullptr) {};
    LRUNode(K k, V v) : key(k), value(v), prev(nullptr), next(nullptr) {};
};

template <typename K, typename V>
class LRUCache {
    using Node = LRUNode<K, V>;
    using n_ptr = typename Node::n_ptr;
    using mem_pool = std::pmr::unsynchronized_pool_resource;
    using mem_alloc = std::pmr::polymorphic_allocator<Node>;

   private:
    n_ptr head;
    n_ptr tail;
    size_t capacity;
    mem_pool pool;
    mem_alloc alloc{&pool};
    std::pmr::unordered_map<K, n_ptr> cache{&pool};

    void disconnect(n_ptr node);
    void add_to_tail(n_ptr node);
    void clear();
    void deallocate(n_ptr node);
    static void sanitise(n_ptr node);

   public:
    explicit LRUCache(size_t capacity);
    ~LRUCache();
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;
    void add(const K& key, const V& value, uint32_t ttl_ms = 0);
    V& get(const K& key);
    void remove(const K& key);
    void remove_expired(uint32_t budget = 0);
};
}  // namespace storage
#include "../../src/storage/lru_cache.tpp"

#endif  // LRU_CACHE_HPP
