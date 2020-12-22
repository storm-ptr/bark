// Andrew Naplavkov

#ifndef BARK_LRU_CACHE_HPP
#define BARK_LRU_CACHE_HPP

#include <atomic>
#include <bark/detail/any_hashable.hpp>
#include <bark/detail/linked_hash_map.hpp>
#include <bark/detail/utility.hpp>
#include <optional>

namespace bark {

/// Thread-safe "Least Recently Used" cache replacement data structure
/// @see https://en.wikipedia.org/wiki/Cache_algorithms#LRU
class lru_cache {
public:
    using scope_type = unsigned long long;
    using key_type = any_hashable;
    using mapped_type = expected<std::any>;
    using hash_type = boost::hash<key_type>;
    using container_type = linked_hash_map<key_type, mapped_type, hash_type>;
    using value_type = container_type::value_type;

    struct busy_exception : std::runtime_error {
        busy_exception() : std::runtime_error{"lru_cache is blocked"} {}
    };

    static scope_type new_scope() { return ++scopes_; }

    template <class Key, class F, class... Args>
    static std::any get_or_invoke(scope_type scope,
                                  const Key& key,
                                  F&& f,
                                  Args&&... args)
    {
        auto scoped_key = key_type{std::make_pair(scope, key)};
        auto guard = timed_lockable<key_type, hash_type>{scoped_key};
        auto lock = std::unique_lock{guard, std::defer_lock};
        if (!lock.try_lock_for(Timeout))
            throw busy_exception{};
        if (auto opt = at(scoped_key))
            return std::move(opt).value().get();
        auto res = mapped_type::result_of(std::forward<F>(f),
                                          std::forward<Args>(args)...);
        insert({scoped_key, res});
        return res.get();
    }

    template <class Key>
    static bool contains(scope_type scope, const Key& key)
    {
        auto lock = std::lock_guard{guard_};
        auto scoped_key = key_type{std::make_pair(scope, key)};
        return data_.find(scoped_key) != data_.end();
    }

private:
    inline static const size_t Capacity{1000};
    inline static const std::chrono::milliseconds Timeout{50};
    inline static std::atomic<scope_type> scopes_{0};
    inline static std::mutex guard_;
    inline static container_type data_;

    static void insert(value_type val)
    {
        auto lock = std::lock_guard{guard_};
        if (!data_.insert(data_.end(), std::move(val)).second)
            throw std::logic_error("lru_cache");
        if (data_.size() > Capacity)
            data_.erase(data_.begin());
    }

    static std::optional<mapped_type> at(const key_type& key)
    {
        auto lock = std::lock_guard{guard_};
        auto it = data_.find(key);
        if (it == data_.end())
            return {};
        data_.move(it, data_.end());
        return it->second;
    }
};

}  // namespace bark

#endif  // BARK_LRU_CACHE_HPP
