// Andrew Naplavkov

#ifndef BARK_LOCKABLE_HPP
#define BARK_LOCKABLE_HPP

#include <boost/functional/hash.hpp>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <unordered_set>

namespace bark {

/**
 * The synchronization primitive offering exclusive ownership of the value of
 * the object (value semantics)
 * @see http://www.cplusplus.com/reference/concept/TimedLockable/
 * @see http://codeofthedamned.com/index.php/value-semantics
 */
template <class T>
class lockable {
public:
    explicit lockable(T key) : key_{std::move(key)} {}
    void lock() { guard().lock(key_); }
    void unlock() { guard().unlock(key_); }
    bool try_lock() { return guard().try_lock(key_); }

    template <class Rep, class Period>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& duration)
    {
        return guard().try_lock_for(key_, duration);
    }

private:
    T key_;

    class lockable_hash_set {
    public:
        void lock(const T& key)
        {
            std::unique_lock lock{guard_};
            notifier_.wait(lock, is_free(key));
            if (!data_.insert(key).second)
                throw std::logic_error("lockable_hash_set");
        }

        void unlock(const T& key)
        {
            {
                std::unique_lock lock{guard_};
                data_.erase(key);
            }
            notifier_.notify_all();
        }

        bool try_lock(const T& key)
        {
            std::unique_lock lock{guard_};
            return data_.insert(key).second;
        }

        template <class Rep, class Period>
        bool try_lock_for(const T& key,
                          const std::chrono::duration<Rep, Period>& duration)
        {
            std::unique_lock lock{guard_};
            return notifier_.wait_for(lock, duration, is_free(key)) &&
                   data_.insert(key).second;
        }

    private:
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

}  // namespace bark

#endif  // BARK_LOCKABLE_HPP
