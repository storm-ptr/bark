// Andrew Naplavkov

#ifndef BARK_UNICODE_HPP
#define BARK_UNICODE_HPP

#include <bark/utility.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <cwctype>
#include <string>
#include <string_view>

namespace bark::unicode {
namespace detail {

template <class Unit>
struct utf8 {
    using decoder = boost::u8_to_u32_iterator<
        typename std::basic_string_view<Unit>::const_iterator>;
    using encoder = boost::u32_to_u8_iterator<std::u32string::const_iterator>;
};

template <class Unit>
struct utf16 {
    using decoder = boost::u16_to_u32_iterator<
        typename std::basic_string_view<Unit>::const_iterator>;
    using encoder = boost::u32_to_u16_iterator<std::u32string::const_iterator>;
};

template <class Unit>
struct utf32 {
    using decoder = typename std::basic_string_view<Unit>::const_iterator;
    using encoder = std::u32string::const_iterator;
};

template <class Unit>
using utf = std::conditional_t<
    sizeof(Unit) == sizeof(char),
    utf8<Unit>,
    std::conditional_t<sizeof(Unit) == sizeof(char16_t),
                       utf16<Unit>,
                       std::conditional_t<sizeof(Unit) == sizeof(char32_t),
                                          utf32<Unit>,
                                          void>>>;

}  // namespace detail

template <class ToUnit, class FromStr>
std::basic_string<ToUnit> to_string(FromStr&& str)
{
    using from_unit = range_value_t<FromStr>;
    using decoder = typename detail::utf<from_unit>::decoder;
    using encoder = typename detail::utf<ToUnit>::encoder;
    auto view = static_cast<std::basic_string_view<from_unit>>(str);
    auto points = std::u32string{decoder{view.begin()}, decoder{view.end()}};
    return {encoder{points.begin()}, encoder{points.end()}};
}

template <class Str>
size_t size(Str&& str)
{
    using unit = range_value_t<Str>;
    using decoder = typename detail::utf<unit>::decoder;
    auto view = static_cast<std::basic_string_view<unit>>(str);
    return std::distance(decoder{view.begin()}, decoder{view.end()});
}

/**
 * @return lowercase version or unmodified if no lowercase version is listed in
 * the current locale
 */
template <class Str>
auto to_lower(Str&& str)
{
    auto wstr = unicode::to_string<wchar_t>(std::forward<Str>(str));
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), std::towlower);
    return unicode::to_string<range_value_t<Str>>(wstr);
}

/**
 * @return uppercase version or unmodified if no uppercase version is listed in
 * the current locale
 */
template <class Str>
auto to_upper(Str&& str)
{
    auto wstr = unicode::to_string<wchar_t>(std::forward<Str>(str));
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), std::towupper);
    return unicode::to_string<range_value_t<Str>>(wstr);
}

struct case_insensitive_equal_to {
    template <class Lhs, class Rhs>
    bool operator()(Lhs&& lhs, Rhs&& rhs) const
    {
        return to_lower(std::forward<Lhs>(lhs)) ==
               to_lower(std::forward<Rhs>(rhs));
    }
};

}  // namespace bark::unicode

#endif  // BARK_UNICODE_HPP
