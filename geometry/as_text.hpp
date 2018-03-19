// Andrew Naplavkov

#ifndef BARK_GEOMETRY_AS_TEXT_HPP
#define BARK_GEOMETRY_AS_TEXT_HPP

#include <bark/common.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/static_visitor.hpp>
#include <ostream>

namespace bark {
namespace geometry {

using boost::geometry::wkt;

namespace detail {

template <typename T>
using wkt_manipulator = decltype(wkt(std::declval<T>()));

struct wkt_manipulator_geometry_collection {
    const geometry_collection& val;

    friend std::ostream& operator<<(
        std::ostream& os,
        const wkt_manipulator_geometry_collection& manip)
    {
        return os << "GEOMETRYCOLLECTION("
                  << list(manip.val, ",", [](auto& item) { return wkt(item); })
                  << ")";
    }
};

using wkt_manipulator_geometry =
    boost::variant<wkt_manipulator<point>,
                   wkt_manipulator<linestring>,
                   wkt_manipulator<polygon>,
                   wkt_manipulator<multi_point>,
                   wkt_manipulator<multi_linestring>,
                   wkt_manipulator<multi_polygon>,
                   wkt_manipulator_geometry_collection>;

struct wkt_visitor : boost::static_visitor<wkt_manipulator_geometry> {
    template <typename T>
    wkt_manipulator_geometry operator()(const T& val) const
    {
        return wkt(val);
    }

    wkt_manipulator_geometry operator()(
        const boost::recursive_wrapper<geometry_collection>& val) const
    {
        return wkt_manipulator_geometry_collection{val.get()};
    }
};

}  // namespace detail

inline auto wkt(const geometry_collection& val)
{
    return detail::wkt_manipulator_geometry_collection{val};
}

inline auto wkt(const geometry& val)
{
    return boost::apply_visitor(detail::wkt_visitor{}, val);
}

template <typename T>
auto as_text(const T& val)
{
    return boost::lexical_cast<std::string>(wkt(val));
}

}  // namespace geometry
}  // namespace bark

#endif  // BARK_GEOMETRY_AS_TEXT_HPP
