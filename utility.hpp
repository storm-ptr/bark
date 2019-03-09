// Andrew Naplavkov

#ifndef BARK_UTILITY_HPP
#define BARK_UTILITY_HPP

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace bark {

constexpr std::chrono::seconds DbTimeout(60);
constexpr std::chrono::milliseconds CacheTimeout(50);
constexpr std::chrono::milliseconds UiTimeout(250);

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

template <class It>
using iter_value_t = typename std::iterator_traits<It>::value_type;

template <class Rng>
using iterator_t = decltype(std::begin(std::declval<Rng>()));

template <class Rng>
using range_value_t = iter_value_t<iterator_t<Rng>>;

template <class T, class Result = void>
using if_arithmetic_t =
    std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, Result>;

template <class T>
if_arithmetic_t<T, T> reversed(T val)
{
    auto first = reinterpret_cast<std::byte*>(&val);
    auto last = first + sizeof(T);
    std::reverse(first, last);
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
auto equal_to(T rhs)
{
    return [rhs](auto&& lhs) { return lhs == rhs; };
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

/**
 * It joins 'items' from a 'range' by adding user defined separator.
 * Additionally there is a version that allows to transform 'items'.
 * @code{.cpp}
 * std::cout << list{{"Hello", "World!"}, ", "};
 * @endcode
 */
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
list(Ts...)->list<Ts...>;

template <class V, class T, size_t I = 0>
constexpr size_t variant_index()
{
    if constexpr (I == std::variant_size_v<V> ||
                  std::is_same_v<std::variant_alternative_t<I, V>, T>)
        return I;
    else
        return variant_index<V, T, I + 1>();
}

}  // namespace bark

#endif  // BARK_UTILITY_HPP
