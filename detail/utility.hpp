// Andrew Naplavkov

#ifndef BARK_UTILITY_HPP
#define BARK_UTILITY_HPP

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <variant>

namespace bark {

constexpr std::chrono::seconds DbTimeout(60);
constexpr std::chrono::milliseconds UiTimeout(250);

/// @see https://en.cppreference.com/w/cpp/utility/variant/visit
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

/// @see https://en.cppreference.com/w/cpp/iterator/iter_t
template <class It>
using iter_value_t = typename std::iterator_traits<It>::value_type;

/// @see https://en.cppreference.com/w/cpp/ranges/iterator_t
template <class Rng>
using iterator_t = decltype(std::begin(std::declval<Rng&>()));

template <class Rng>
using range_value_t = iter_value_t<iterator_t<Rng>>;

template <class T, class Result = void>
using if_arithmetic_t =
    std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, Result>;

template <class T>
if_arithmetic_t<T, T> reversed(T val)
{
    if constexpr (sizeof(T) > 1) {
        auto first = reinterpret_cast<std::byte*>(&val);
        auto last = first + sizeof(T);
        std::reverse(first, last);
    }
    return val;
}

inline auto within(std::string_view rhs)
{
    return [rhs](std::string_view lhs) {
        return rhs.find(lhs) != std::string_view::npos;
    };
}

/// @see https://youtu.be/qL6zUn7iiLg?t=477
template <class T>
auto equals(const T& lhs)
{
    return [lhs](const auto& rhs) { return lhs == rhs; };
}

template <class T, class Predicate>
bool any_of(std::initializer_list<T> rng, Predicate p)
{
    return std::any_of(rng.begin(), rng.end(), p);
}

template <class T, class Predicate>
bool all_of(std::initializer_list<T> rng, Predicate p)
{
    return std::all_of(rng.begin(), rng.end(), p);
}

template <class Container, class Item>
void resize_and_assign(Container& container, size_t pos, Item&& item)
{
    if (container.size() <= pos)
        container.resize(pos + 1);
    container[pos] = std::forward<Item>(item);
}

/// @see https://en.cppreference.com/w/cpp/utility/functional/identity
struct identity {
    template <class T>
    constexpr decltype(auto) operator()(T&& val) const noexcept
    {
        return std::forward<T>(val);
    }
};

template <class Result, class Rng, class Operation = identity>
auto as(const Rng& rng, Operation op = {})
{
    Result res;
    std::transform(std::begin(rng), std::end(rng), std::back_inserter(res), op);
    return res;
}

template <class V, class T, size_t I = 0>
constexpr size_t variant_index()
{
    if constexpr (I == std::variant_size_v<V> ||
                  std::is_same_v<std::variant_alternative_t<I, V>, T>)
        return I;
    else
        return variant_index<V, T, I + 1>();
}

/// Synchronous std::future<T> analogue.
/// expected<T> is either a T or the exception preventing its creation.
template <class T>
class expected {
public:
    template <class F, class... Args>
    static expected result_of(F&& f, Args&&... args) noexcept
    {
        expected res;
        try {
            res.state_ =
                std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        }
        catch (const std::exception&) {
            res.state_ = std::current_exception();
        }
        return res;
    }

    T get()
    {
        return std::visit(overloaded{[](T& val) -> T { return std::move(val); },
                                     [](std::exception_ptr& e) -> T {
                                         std::rethrow_exception(std::move(e));
                                     }},
                          state_);
    }

private:
    std::variant<std::exception_ptr, T> state_;
};

/// Joins 'items' by adding user defined separator.
/// @code std::cout << list{{"Hello", "World!"}, ", "}; @endcode
template <class Rng, class Separator, class Operation = identity>
struct list {
    const Rng& rng;
    const Separator& sep;
    Operation op;

    template <class OStream>
    friend OStream& operator<<(OStream& os, const list& that)
    {
        auto first = std::begin(that.rng);
        auto last = std::end(that.rng);
        if (first != last)
            os << that.op(*first++);
        while (first != last)
            os << that.sep << that.op(*first++);
        return os;
    }
};

template <class... Ts>
list(Ts...) -> list<Ts...>;

class random_index {
public:
    using generator_type = std::mt19937;
    using value_type = generator_type::result_type;

    explicit random_index(value_type size)
        : gen_(std::random_device()()), dist_(0, size - 1)
    {
    }

    value_type operator()()
    {
        auto lock = std::lock_guard{guard_};
        return dist_(gen_);
    }

private:
    std::mutex guard_;
    generator_type gen_;
    std::uniform_int_distribution<value_type> dist_;
};

template <class Type>
struct same {
    Type type;

    template <class T>
    bool operator()(const T& val) const
    {
        return type == val.type;
    }
};

template <class T>
same(T) -> same<T>;

template <class Functor>
struct streamable : Functor {
    explicit streamable(Functor&& f) : Functor{std::move(f)} {}

    template <class OStream>
    friend OStream& operator<<(OStream& os, const streamable& that)
    {
        that(os);
        return os;
    }
};

template <class T>
streamable(T) -> streamable<T>;

/// Synchronization primitive for value semantics.
/// @see https://en.cppreference.com/w/cpp/named_req/TimedLockable
template <class T, class Hash = std::hash<T>, class Equal = std::equal_to<T>>
class timed_lockable {
public:
    explicit timed_lockable(const T& val) : val_{val} {}

    template <class Rep, class Period>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& duration)
    {
        auto lock = std::unique_lock{guard_};
        return notifier_.wait_for(lock, duration, [&] {
            return !locked_.count(val_);
        }) && locked_.insert(val_).second;
    }

    void unlock()
    {
        {
            auto lock = std::unique_lock{guard_};
            locked_.erase(val_);
        }
        notifier_.notify_all();
    }

private:
    T val_;
    inline static std::mutex guard_;
    inline static std::condition_variable notifier_;
    inline static std::unordered_set<T, Hash, Equal> locked_;
};

}  // namespace bark

#endif  // BARK_UTILITY_HPP
