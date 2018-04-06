// Andrew Naplavkov

#ifndef BARK_COMMON_HPP
#define BARK_COMMON_HPP

#include <algorithm>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/version.hpp>
#include <chrono>
#include <cstdint>
#include <initializer_list>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <type_traits>
#include <vector>

#if (BOOST_VERSION < 106100)
#include <boost/utility/string_ref.hpp>
#else
#include <boost/utility/string_view.hpp>
#endif

namespace bark {

template <typename charT>
using basic_string_view =
#if (BOOST_VERSION < 106100)
    boost::basic_string_ref
#else
    boost::basic_string_view
#endif
    <typename charT>;

/// Binary Large OBject (BLOB) - contiguous byte storage that can change in size
struct blob_t : std::vector<uint8_t> {
    using std::vector<uint8_t>::vector;
};

struct blob_view : basic_string_view<uint8_t> {
    using basic_string_view<uint8_t>::basic_string_view;

    blob_view(const uint8_t*) = delete;

    blob_view(const blob_t& blob)
        : basic_string_view<uint8_t>(blob.data(), blob.size())
    {
    }
};

using string_view = basic_string_view<char>;

constexpr std::chrono::seconds DbTimeout(60);
constexpr std::chrono::milliseconds CacheTimeout(50);
constexpr std::chrono::milliseconds UiTimeout(250);

template <typename T>
std::enable_if_t<std::is_arithmetic<T>::value, T> reversed(T val)
{
    auto ptr = reinterpret_cast<uint8_t*>(&val);
    std::reverse(ptr, ptr + sizeof(T));
    return val;
}

inline auto within(string_view rhs)
{
    return
        [rhs](string_view lhs) { return rhs.find(lhs) != std::string::npos; };
}

inline auto equal_to(string_view rhs)
{
    return [rhs](string_view lhs) { return lhs == rhs; };
}

template <typename T, typename Predicate>
bool any_of(std::initializer_list<T> rng, Predicate pred)
{
    return std::any_of(rng.begin(), rng.end(), pred);
}

template <typename T, typename Predicate>
bool all_of(std::initializer_list<T> rng, Predicate pred)
{
    return std::all_of(rng.begin(), rng.end(), pred);
}

template <typename RandomAccessRange, typename Functor>
void for_each_slice(RandomAccessRange&& rng, size_t slice_size, Functor fn)
{
    for (size_t pos = 0; pos < boost::size(rng);) {
        auto begin = pos;
        pos += std::min(boost::size(rng) - pos, slice_size);
        fn(boost::adaptors::slice(rng, begin, pos));
    }
}

template <typename Container, typename Item>
void resize_and_assign(Container& container, size_t pos, Item&& item)
{
    if (container.size() <= pos)
        container.resize(pos + 1);
    container[pos] = std::forward<Item>(item);
}

template <typename Result, typename Range, typename Operation>
auto back_constructor(const Range& rng, Operation op)
{
    Result res;
    std::transform(rng.begin(), rng.end(), std::back_inserter(res), op);
    return res;
}

namespace detail {

template <typename Range, typename Separator, typename Operation>
struct list_manipulator {
    const Range& rng;
    const Separator& sep;
    Operation op;

    template <typename OStream>
    friend OStream& operator<<(OStream& os, const list_manipulator& manip)
    {
        auto begin = std::begin(manip.rng);
        auto end = std::end(manip.rng);
        for (auto it = begin; it != end; ++it) {
            if (it != begin)
                os << manip.sep;
            os << manip.op(*it);
        }
        return os;
    }
};

}  // namespace detail

/**
 * It joins 'items' from a 'range' by adding user defined separator.
 * Additionally there is a version that allows to transform 'items'.
 * @code{.cpp}
 * std::cout << list({"Hello", "World!"}, ", ");
 * @endcode
 */
template <typename Range, typename Separator, typename Operation>
auto list(const Range& rng, const Separator& sep, Operation op)
{
    return bark::detail::list_manipulator<Range, Separator, Operation>{
        rng, sep, op};
}

template <typename Range, typename Separator>
auto list(const Range& rng, const Separator& sep)
{
    return list(rng, sep, [](auto& item) -> decltype(auto) { return item; });
}

/// @return true if T is equal to any of type in Ts list
template <typename T, typename... Ts>
constexpr bool is_same()
{
    return boost::mpl::contains<boost::mpl::vector<Ts...>, T>::value;
}

/// @return a 0-based index of type T in a Variant::types
template <typename Variant, typename T>
constexpr std::enable_if_t<
    boost::mpl::contains<typename Variant::types, T>::value,
    uint8_t>
which()
{
    using namespace boost::mpl;
    using types =
        typename copy<typename Variant::types, back_inserter<vector<>>>::type;
    using iterator = typename find<types, T>::type;
    return static_cast<uint8_t>(iterator::pos::value);
}

template <typename T>
std::enable_if_t<is_same<T, blob_t, blob_view>(), std::ostream&> operator<<(
    std::ostream& os,
    const T& blob)
{
    return os << (std::to_string(blob.size()) + " bytes");  // call stream once
}

namespace detail {

struct hex_manipulator {
    blob_view blob;

    friend std::ostream& operator<<(std::ostream& os,
                                    const hex_manipulator& manip)
    {
        std::ostringstream ss;
        ss << std::uppercase << std::hex << std::setfill('0');
        for (uint8_t byte : manip.blob)
            ss << std::setw(2) << static_cast<unsigned>(byte);
        return os << ss.str();  // call stream once
    }
};

}  // namespace detail

inline auto hex(blob_view blob)
{
    return detail::hex_manipulator{blob};
}

}  // namespace bark

#endif  // BARK_COMMON_HPP
