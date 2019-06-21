// Andrew Naplavkov

#ifndef BARK_GEOMETRY_AS_TEXT_HPP
#define BARK_GEOMETRY_AS_TEXT_HPP

#include <bark/detail/utility.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/lexical_cast.hpp>
#include <ostream>

namespace bark::geometry {

/// I/O manipulator.

/// @see
/// https://en.wikipedia.org/wiki/Well-known_text_representation_of_geometry
template <class T>
struct wkt {
    const T& val;

    friend std::ostream& operator<<(std::ostream& os, const wkt& that)
    {
        return os << boost::geometry::wkt(that.val);
    }
};

template <class T>
wkt(T)->wkt<T>;

namespace detail {

struct make_wkt {
    template <class T>
    constexpr auto operator()(const T& val) const
    {
        return wkt<T>{val};
    };
};

}  // namespace detail

template <>
struct wkt<geometry_collection> {
    const geometry_collection& val;

    friend std::ostream& operator<<(std::ostream& os, const wkt& that)
    {
        return os << "GEOMETRYCOLLECTION("
                  << list{that.val, ",", detail::make_wkt{}} << ")";
    }
};

template <>
struct wkt<geometry> {
    const geometry& val;

    friend std::ostream& operator<<(std::ostream& os, const wkt& that)
    {
        boost::apply_visitor(
            [&](const auto& val) { os << detail::make_wkt{}(val); }, that.val);
        return os;
    }
};

template <class T>
auto as_text(const T& val)
{
    return boost::lexical_cast<std::string>(wkt{val});
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_AS_TEXT_HPP
