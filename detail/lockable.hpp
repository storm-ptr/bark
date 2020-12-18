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

/// Synchronization primitive for value semantics.
/// @see http://www.cplusplus.com/reference/concept/TimedLockable/
/// @see http://codeofthedamned.com/index.php/value-semantics
template <class T>
class lockable {
public:
    explicit lockable(const T& val) : val_{val} {}
    void lock() { guard().lock(val_); }
    void unlock() { guard().unlock(val_); }
    bool try_lock() { return guard().try_lock(val_); }

    template <class Rep, class Period>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& duration)
    {
        return guard().try_lock_for(val_, duration);
    }

private:
    T val_;

    class lockable_hash_set {
    public:
        void lock(const T& val)
        {
            std::unique_lock lock{guard_};
            notifier_.wait(lock, vacant(val));
            if (!data_.insert(val).second)
                throw std::logic_error("lockable_hash_set");
        }

        void unlock(const T& val)
        {
            {
                std::unique_lock lock{guard_};
                data_.erase(val);
            }
            notifier_.notify_all();
        }

        bool try_lock(const T& val)
        {
            std::unique_lock lock{guard_};
            return data_.insert(val).second;
        }

        template <class Rep, class Period>
        bool try_lock_for(const T& val,
                          const std::chrono::duration<Rep, Period>& duration)
        {
            std::unique_lock lock{guard_};
            return notifier_.wait_for(lock, duration, vacant(val)) &&
                   data_.insert(val).second;
        }

    private:
        std::mutex guard_;
        std::condition_variable notifier_;
        std::unordered_set<T, boost::hash<T>> data_;

        auto vacant(const T& val) const
        {
            return [=] { return data_.find(val) == data_.end(); };
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
