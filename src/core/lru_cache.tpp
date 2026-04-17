//
// Created by Yeo Shu Heng on 14/4/26.
//

#include <spdlog/spdlog.h>

#include <format>

namespace core {
constexpr uint32_t EVICT_BUDGET = 10;
template <typename K, typename V>
LRUCache<K, V>::LRUCache(const size_t capacity) : capacity(capacity) {
    if (capacity == 0)
        throw std::runtime_error("lru cache must have at least capacity 1");
    head = alloc.allocate(1);
    alloc.construct(head);

    tail = alloc.allocate(1);
    alloc.construct(tail);

    head->next = tail;
    tail->prev = head;
};

template <typename K, typename V> LRUCache<K, V>::~LRUCache() {
    for (auto& [_, node] : cache) {
        deallocate(node);
    }
    cache.clear();
    deallocate(head);
    deallocate(tail);
};

template <typename K, typename V> void LRUCache<K, V>::disconnect(n_ptr node) {
    if (node == nullptr) {
        return;
    }
    const auto n_prev = node->prev;
    const auto n_next = node->next;
    if (n_prev != nullptr) {
        n_prev->next = n_next;
    }
    if (n_next != nullptr) {
        n_next->prev = n_prev;
    }
    sanitise(node);
};

template <typename K, typename V> void LRUCache<K, V>::evict_till_capacity() {
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

template <typename K, typename V> void LRUCache<K, V>::add_to_tail(n_ptr node) {
    const auto t_prev = tail->prev;
    t_prev->next = node;
    node->prev = t_prev;
    tail->prev = node;
    node->next = tail;
};

template <typename K, typename V> void LRUCache<K, V>::sanitise(n_ptr node) {
    if (node == nullptr) {
        return;
    }
    node->prev = node->next = nullptr;
};

template <typename K, typename V> void LRUCache<K, V>::deallocate(n_ptr node) {
    alloc.destroy(node);
    alloc.deallocate(node, 1);
};

template <typename K, typename V>
void LRUCache<K, V>::add(const K& key, const V& value, const uint32_t ttl_ms) {
    n_ptr node;
    const auto it = cache.find(key);
    if (it != cache.end()) {
        node = it->second;
        node->value = value;
        node->expired_t.reset();
        disconnect(node);
    } else {
        remove_expired(EVICT_BUDGET);
        evict_till_capacity();
        node = alloc.allocate(1);
        alloc.construct(node, key, value);
        cache.emplace(key, node);
    }
    if (ttl_ms > 0) {
        node->expired_t = Clock::now() + std::chrono::milliseconds(ttl_ms);
    }
    add_to_tail(node);
};

template <typename K, typename V> std::optional<V> LRUCache<K, V>::get(const K& key) {
    const auto it = cache.find(key);
    if (it == cache.end()) {
        spdlog::debug("cannot find key: {}", key);
        return std::nullopt;
    }
    const auto node = it->second;
    if (node->expired_t.has_value() && Clock::now() >= *node->expired_t) {
        spdlog::debug("node with key: {} has expired", key);
        cache.erase(it);
        disconnect(node);
        deallocate(node);
        return std::nullopt;
    }
    disconnect(node);
    add_to_tail(node);
    return node->value;
};

template <typename K, typename V> bool LRUCache<K, V>::remove(const K& key) {
    const auto it = cache.find(key);
    if (it == cache.end()) {
        spdlog::debug("cannot find key: {}", key);
        return false;
    }
    const auto node = it->second;
    disconnect(node);
    cache.erase(key);
    deallocate(node);
    return true;
};

template <typename K, typename V> void LRUCache<K, V>::remove_expired(const uint32_t budget) {
    const uint32_t limit = (budget == 0) ? std::numeric_limits<uint32_t>::max() : budget;

    auto now = Clock::now();
    uint32_t consumed = 0;

    for (auto it = cache.begin(); it != cache.end() && consumed < limit;) {
        if (auto node = it->second; node->expired_t.has_value() && *node->expired_t <= now) {
            cache.erase(it++);
            disconnect(node);
            deallocate(node);
            ++consumed;
        } else {
            ++it;
        }
    }
}
} // namespace core
