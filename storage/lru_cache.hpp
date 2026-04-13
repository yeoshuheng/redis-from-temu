//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <unordered_map>

namespace storage {
template <typename K, typename V>
struct LRUNode {
    using n_ptr = LRUNode<K, V>*;

    K key;
    V value;
    n_ptr prev;
    n_ptr next;

    LRUNode() : key(), value(), prev(nullptr), next(nullptr) {};
    LRUNode(K k, V v) : key(k), value(v), prev(nullptr), next(nullptr) {};
};

template <typename K, typename V>
class LRUCache {
    using Node = LRUNode<K, V>;
    using n_ptr = typename Node::n_ptr;

   private:
    n_ptr head;
    n_ptr tail;
    std::unordered_map<K, n_ptr> cache;
    size_t capacity;

    void disconnect(n_ptr node);
    void add_to_tail(n_ptr node);
    void clear();

    static void sanitise(n_ptr node);

   public:
    explicit LRUCache(size_t capacity);
    void add(const K& key, const V& value);
    [[nodiscard]] V get(const K& key);
    void remove(const K& key);
};
}  // namespace storage

#endif  // LRU_CACHE_HPP
