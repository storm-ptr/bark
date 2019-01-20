// Andrew Naplavkov

#ifndef BARK_LRU_CACHE_HPP
#define BARK_LRU_CACHE_HPP

#include <any>
#include <atomic>
#include <bark/detail/any_hashable.hpp>
#include <bark/detail/expected.hpp>
#include <bark/detail/linked_hash_map.hpp>
#include <bark/detail/lockable.hpp>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace bark {

/**
 * thread-safe "Least Recently Used" cache replacement data structure
 * @see https://en.wikipedia.org/wiki/Cache_algorithms#LRU
 */
class lru_cache {
    static auto& one()
    {
        static lru_cache singleton{};
        return singleton;
    }

public:
    using scope_type = uint64_t;
    using key_type = any_hashable;
    using mapped_type = expected<std::any>;
    using value_type = std::pair<const key_type, mapped_type>;
    using loader_type = std::function<std::any()>;

    struct busy_exception : std::runtime_error {
        busy_exception() : std::runtime_error{"lru_cache: blocked"} {}
    };

    static scope_type next_scope() { return ++one().max_scope_; }

    template <class T>
    static std::any get_or_load(scope_type scope, T key, loader_type loader)
    {
        auto key_pair = make_key_pair(scope, std::move(key));
        auto mutex = make_lockable(key_pair);
        std::unique_lock<decltype(mutex)> lock{mutex, std::defer_lock};
        if (!lock.try_lock_for(CacheTimeout))
            throw busy_exception();
        auto optional = one().at(key_pair);
        if (optional)
            return optional->get();
        auto expected = make_expected(loader);
        one().insert({key_pair, expected});
        return expected.get();
    }

    template <class T>
    static bool contains(scope_type scope, T key)
    {
        return one().has(make_key_pair(scope, std::move(key)));
    }

private:
    static constexpr size_t MaxSize{10000};
    std::atomic<scope_type> max_scope_{0};
    std::mutex guard_;
    linked_hash_map<key_type, mapped_type> data_;

    template <class T>
    static auto make_key_pair(scope_type scope, T key)
    {
        return key_type{std::make_pair(scope, std::move(key))};
    }

    bool has(const key_type& key)
    {
        std::lock_guard lock{guard_};
        return data_.find(key) != data_.end();
    }

    void insert(const value_type& val)
    {
        std::lock_guard lock{guard_};
        if (!data_.insert(data_.end(), val).second)
            throw std::logic_error("lru_cache: already exists");
        if (data_.size() > MaxSize)
            data_.pop_front();
    }

    std::optional<mapped_type> at(const key_type& key)
    {
        std::lock_guard lock{guard_};
        auto pos = data_.find(key);
        if (pos == data_.end())
            return {};
        auto optional = pos->second;
        data_.move(pos, data_.end());
        return optional;
    }
};

}  // namespace bark

#endif  // BARK_LRU_CACHE_HPP
