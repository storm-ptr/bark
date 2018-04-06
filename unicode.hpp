// Andrew Naplavkov

#ifndef BARK_UNICODE_HPP
#define BARK_UNICODE_HPP

#include <algorithm>
#include <bark/common.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <cwctype>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>

namespace bark {
namespace unicode {
namespace detail {

template <typename Str>
using unit_t = typename std::iterator_traits<decltype(
    std::begin((std::declval<Str>())))>::value_type;

template <typename Unit>
struct utf8 {
    static_assert(sizeof(Unit) == sizeof(char), "unicode size");
    using decoder = boost::u8_to_u32_iterator<
        typename basic_string_view<Unit>::const_iterator>;
    using encoder = boost::u32_to_u8_iterator<std::u32string::const_iterator>;
};

template <typename Unit>
struct utf16 {
    static_assert(sizeof(Unit) == sizeof(char16_t), "unicode size");
    using decoder = boost::u16_to_u32_iterator<
        typename basic_string_view<Unit>::const_iterator>;
    using encoder = boost::u32_to_u16_iterator<std::u32string::const_iterator>;
};

template <typename Unit>
struct utf32 {
    static_assert(sizeof(Unit) == sizeof(char32_t), "unicode size");
    using decoder = typename basic_string_view<Unit>::const_iterator;
    using encoder = std::u32string::const_iterator;
};

template <typename Unit>
using utf = typename std::conditional_t<
    sizeof(Unit) == sizeof(char),
    utf8<Unit>,
    std::conditional_t<sizeof(Unit) == sizeof(char16_t),
                       utf16<Unit>,
                       std::conditional_t<sizeof(Unit) == sizeof(char32_t),
                                          utf32<Unit>,
                                          void>>>;

}  // namespace detail

template <typename ToUnit, typename Str, typename Unit = detail::unit_t<Str>>
std::basic_string<ToUnit> to_string(Str&& str)
{
    using decoder = typename detail::utf<Unit>::decoder;
    using encoder = typename detail::utf<ToUnit>::encoder;
    auto view = basic_string_view<Unit>{std::forward<Str>(str)};
    auto points = std::u32string{decoder{view.begin()}, decoder{view.end()}};
    return {encoder{points.begin()}, encoder{points.end()}};
}

/**
 * @return lowercase version or unmodified if no lowercase version is listed in
 * the current locale
 */
template <typename Str, typename Unit = detail::unit_t<Str>>
std::basic_string<Unit> to_lower(Str&& str)
{
    auto wstr = unicode::to_string<wchar_t>(std::forward<Str>(str));
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), towlower);
    return unicode::to_string<Unit>(wstr);
}

/**
 * @return uppercase version or unmodified if no uppercase version is listed in
 * the current locale
 */
template <typename Str, typename Unit = detail::unit_t<Str>>
std::basic_string<Unit> to_upper(Str&& str)
{
    auto wstr = unicode::to_string<wchar_t>(std::forward<Str>(str));
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), towupper);
    return unicode::to_string<Unit>(wstr);
}

struct case_insensitive_equal_to {
    template <typename Lhs, typename Rhs>
    bool operator()(Lhs&& lhs, Rhs&& rhs) const
    {
        return unicode::to_lower(std::forward<Lhs>(lhs)) ==
               unicode::to_lower(std::forward<Rhs>(rhs));
    }
};

}  // namespace unicode
}  // namespace bark

#endif  // BARK_UNICODE_HPP
