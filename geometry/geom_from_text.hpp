// Andrew Naplavkov

#ifndef BARK_GEOMETRY_GEOM_FROM_TEXT_HPP
#define BARK_GEOMETRY_GEOM_FROM_TEXT_HPP

#include <bark/geometry/detail/grammar.hpp>
#include <bark/geometry/geometry.hpp>
#include <stdexcept>
#include <string_view>

namespace bark::geometry {
namespace detail {

template <class T>
T parse_wkt(std::string_view wkt)
{
    T res;
    grammar<T> gr;
    return qi::phrase_parse(
               wkt.data(), wkt.data() + wkt.size(), gr, qi::blank, res)
               ? res
               : throw std::runtime_error(std::string{wkt});
}

}  // namespace detail

inline auto geom_from_text(std::string_view wkt)
{
    return detail::parse_wkt<geometry>(wkt);
}

inline auto point_from_text(std::string_view wkt)
{
    return detail::parse_wkt<point>(wkt);
}

inline auto line_from_text(std::string_view wkt)
{
    return detail::parse_wkt<linestring>(wkt);
}

inline auto poly_from_text(std::string_view wkt)
{
    return detail::parse_wkt<polygon>(wkt);
}

inline auto mpoint_from_text(std::string_view wkt)
{
    return detail::parse_wkt<multi_point>(wkt);
}

inline auto mline_from_text(std::string_view wkt)
{
    return detail::parse_wkt<multi_linestring>(wkt);
}

inline auto mpoly_from_text(std::string_view wkt)
{
    return detail::parse_wkt<multi_polygon>(wkt);
}

inline auto geom_coll_from_text(std::string_view wkt)
{
    return detail::parse_wkt<geometry_collection>(wkt);
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_GEOM_FROM_TEXT_HPP
