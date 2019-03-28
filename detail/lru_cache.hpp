// Andrew Naplavkov

#ifndef BARK_LRU_CACHE_HPP
#define BARK_LRU_CACHE_HPP

#include <any>
#include <atomic>
#include <bark/detail/any_hashable.hpp>
#include <bark/detail/linked_hash_map.hpp>
#include <bark/detail/lockable.hpp>
#include <cstdint>
#include <mutex>
#include <optional>

namespace bark {

/**
 * thread-safe "Least Recently Used" cache replacement data structure
 * @see https://en.wikipedia.org/wiki/Cache_algorithms#LRU
 */
class lru_cache {
public:
    using scope_type = uint64_t;
    using container_type = linked_hash_map<any_hashable, expected<std::any>>;
    using key_type = container_type::key_type;
    using mapped_type = container_type::mapped_type;
    using value_type = container_type::value_type;

    struct busy_exception : std::runtime_error {
        busy_exception() : std::runtime_error{"lru_cache is blocked"} {}
    };

    static scope_type new_scope() { return ++one().scopes_; }

    template <class Key, class F, class... Args>
    static std::any get_or_invoke(scope_type scope,
                                  const Key& key,
                                  F&& f,
                                  Args&&... args)
    {
        key_type scoped_key{std::make_pair(scope, key)};
        lockable guard{scoped_key};
        std::unique_lock lock{guard, std::defer_lock};
        if (!lock.try_lock_for(CacheTimeout))
            throw busy_exception{};
        if (auto opt = one().at(scoped_key))
            return std::move(opt).value().get();
        auto res = mapped_type::result_of(std::forward<F>(f),
                                          std::forward<Args>(args)...);
        one().insert({scoped_key, res});
        return res.get();
    }

    template <class Key>
    static bool contains(scope_type scope, const Key& key)
    {
        key_type scoped_key{std::make_pair(scope, key)};
        return one().contains(scoped_key);
    }

private:
    static constexpr size_t MaxSize{10000};
    std::atomic<scope_type> scopes_{0};
    std::mutex guard_;
    container_type data_;

    static lru_cache& one()
    {
        static lru_cache singleton{};
        return singleton;
    }

    bool contains(const key_type& key)
    {
        std::lock_guard lock{guard_};
        return data_.find(key) != data_.end();
    }

    void insert(value_type val)
    {
        std::lock_guard lock{guard_};
        if (!data_.insert(data_.end(), std::move(val)).second)
            throw std::logic_error("lru_cache");
        if (data_.size() > MaxSize)
            data_.erase(data_.begin());
    }

    std::optional<mapped_type> at(const key_type& key)
    {
        std::lock_guard lock{guard_};
        auto it = data_.find(key);
        if (it == data_.end())
            return {};
        data_.move(it, data_.end());
        return it->second;
    }
};

}  // namespace bark

#endif  // BARK_LRU_CACHE_HPP
