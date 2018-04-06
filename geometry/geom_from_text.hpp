// Andrew Naplavkov

#ifndef BARK_GEOMETRY_GEOM_FROM_TEXT_HPP
#define BARK_GEOMETRY_GEOM_FROM_TEXT_HPP

#include <bark/common.hpp>
#include <bark/geometry/detail/grammar.hpp>
#include <bark/geometry/geometry.hpp>
#include <stdexcept>

namespace bark {
namespace geometry {
namespace detail {

template <typename T>
T parse_wkt(string_view wkt)
{
    grammar<T> gr;
    T res;
    if (!qi::phrase_parse(wkt.begin(), wkt.end(), gr, qi::blank, res))
        throw std::runtime_error(wkt.to_string());
    return res;
}

}  // namespace detail

inline auto geom_from_text(string_view wkt)
{
    return detail::parse_wkt<geometry>(wkt);
}

inline auto point_from_text(string_view wkt)
{
    return detail::parse_wkt<point>(wkt);
}

inline auto line_from_text(string_view wkt)
{
    return detail::parse_wkt<linestring>(wkt);
}

inline auto poly_from_text(string_view wkt)
{
    return detail::parse_wkt<polygon>(wkt);
}

inline auto mpoint_from_text(string_view wkt)
{
    return detail::parse_wkt<multi_point>(wkt);
}

inline auto mline_from_text(string_view wkt)
{
    return detail::parse_wkt<multi_linestring>(wkt);
}

inline auto mpoly_from_text(string_view wkt)
{
    return detail::parse_wkt<multi_polygon>(wkt);
}

inline auto geom_coll_from_text(string_view wkt)
{
    return detail::parse_wkt<geometry_collection>(wkt);
}

}  // namespace geometry
}  // namespace bark

#endif  // BARK_GEOMETRY_GEOM_FROM_TEXT_HPP
