// Andrew Naplavkov

#ifndef BARK_DETAIL_LOCKABLE_HPP
#define BARK_DETAIL_LOCKABLE_HPP

#include <boost/core/noncopyable.hpp>
#include <boost/functional/hash.hpp>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <unordered_set>

namespace bark {
namespace detail {

/**
 * The synchronization primitive offering exclusive ownership of the value of
 * the object (value semantics)
 * @see http://www.cplusplus.com/reference/concept/TimedLockable/
 * @see http://codeofthedamned.com/index.php/value-semantics
 */
template <typename T>
class lockable {
public:
    explicit lockable(T key) : key_{std::move(key)} {}

    void lock() { guard().lock(key_); }

    bool try_lock() { return guard().try_lock(key_); }

    template <class Rep, class Period>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& duration)
    {
        return guard().try_lock_for(key_, duration);
    }

    void unlock() { guard().unlock(key_); }

private:
    const T key_;

    class lockable_hash_set : private boost::noncopyable {
    public:
        void lock(const T& key)
        {
            lock_t lock{guard_};
            notifier_.wait(lock, is_free(key));
            if (!data_.insert(key).second)
                throw std::logic_error("lockable_hash_set");
        }

        bool try_lock(const T& key)
        {
            lock_t lock{guard_};
            return data_.insert(key).second;
        }

        template <class Rep, class Period>
        bool try_lock_for(const T& key,
                          const std::chrono::duration<Rep, Period>& duration)
        {
            lock_t lock{guard_};
            return notifier_.wait_for(lock, duration, is_free(key)) &&
                   data_.insert(key).second;
        }

        void unlock(const T& key)
        {
            {
                lock_t lock{guard_};
                data_.erase(key);
            }
            notifier_.notify_all();
        }

    private:
        using lock_t = std::unique_lock<std::mutex>;

        std::mutex guard_;
        std::condition_variable notifier_;
        std::unordered_set<T, boost::hash<T>> data_;

        auto is_free(const T& key) const
        {
            return [key, this] { return data_.find(key) == data_.end(); };
        }
    };

    static auto& guard()
    {
        static lockable_hash_set singleton{};
        return singleton;
    }
};

template <typename T>
auto make_lockable(T key)
{
    return lockable<T>{key};
}

}  // namespace detail
}  // namespace bark

#endif  // BARK_DETAIL_LOCKABLE_HPP
