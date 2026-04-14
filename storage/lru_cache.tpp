//
// Created by Yeo Shu Heng on 14/4/26.
//

#include <format>
#include <spdlog/spdlog.h>

namespace storage {
template <typename K, typename V>
LRUCache<K, V>::LRUCache(const size_t capacity)
    : capacity(capacity) {

    head = alloc.allocate(1);
    alloc.construct(head);

    tail = alloc.allocate(1);
    alloc.construct(tail);

    head->next = tail;
    tail->prev = head;
};

template <typename K, typename V>
LRUCache<K, V>::~LRUCache() {
    for (auto& [_, node] : cache) {
        deallocate(node);
    }
    deallocate(head);
    deallocate(tail);
    cache.clear();
};

template <typename K, typename V>
void LRUCache<K, V>::disconnect(n_ptr node) {
    if (node == nullptr) {
        return;
    }
    const auto n_prev = node->prev;
    const auto n_next = node->next;
    sanitise(node);
    if (n_prev != nullptr) {
        n_prev->next = n_next;
    }
    if (n_next != nullptr) {
        n_next->prev = n_prev;
    }
};

template <typename K, typename V>
void LRUCache<K, V>::clear() {
    while (cache.size() >= capacity) {
        const auto to_evict = head->next;
        if (to_evict == tail) {
            break;
        }
        cache.erase(to_evict->key);
        disconnect(to_evict);
        deallocate(to_evict);
    }
}

template <typename K, typename V>
void LRUCache<K, V>::add_to_tail(n_ptr node) {
    const auto t_prev = tail->prev;
    t_prev->next = node;
    node->prev = t_prev;
    tail->prev = node;
    node->next = tail;
};

template <typename K, typename V>
void LRUCache<K, V>::sanitise(n_ptr node) {
    if (node == nullptr) {
        return;
    }
    node->prev = node->next = nullptr;
};

template <typename K, typename V>
void LRUCache<K, V>::deallocate(n_ptr node) {
    alloc.destroy(node);
    alloc.deallocate(node, 1);
};

template <typename K, typename V>
void LRUCache<K, V>::add(const K& key, const V& value) {
    n_ptr node;
    const auto it = cache.find(key);
    if (it != cache.end()) {
        node = it->second;
        node->value = value;
        disconnect(node);
    } else {
        clear();
        node = alloc.allocate(1);
        alloc.construct(node, key, value);
        cache.emplace(key, node);
    }
    add_to_tail(node);
};

template <typename K, typename V>
V LRUCache<K, V>::get(const K& key) {
    const auto it = cache.find(key);
    if (it == cache.end()) {
        spdlog::error("cannot find key: {}", key);
        throw std::runtime_error(std::format("unable to find node with key: {}", key));
    }
    const auto node = it->second;
    disconnect(node);
    add_to_tail(node);
    return node->value;
};

template <typename K, typename V>
void LRUCache<K, V>::remove(const K& key) {
    const auto it = cache.find(key);
    if (it == cache.end()) {
        spdlog::error("cannot find key: {}", key);
        throw std::runtime_error(std::format("unable to find node with key: {}", key));
    }
    const auto node = it->second;
    disconnect(node);
    cache.erase(key);
    deallocate(node);
};
}  // namespace storage
