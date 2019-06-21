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
T parse_wkt(std::string_view v)
{
    T res;
    grammar<T> gr;
    return qi::phrase_parse(v.data(), v.data() + v.size(), gr, qi::blank, res)
               ? res
               : throw std::runtime_error(std::string{v});
}

}  // namespace detail

inline auto geom_from_text(std::string_view v)
{
    return detail::parse_wkt<geometry>(v);
}

inline auto point_from_text(std::string_view v)
{
    return detail::parse_wkt<point>(v);
}

inline auto line_from_text(std::string_view v)
{
    return detail::parse_wkt<linestring>(v);
}

inline auto poly_from_text(std::string_view v)
{
    return detail::parse_wkt<polygon>(v);
}

inline auto mpoint_from_text(std::string_view v)
{
    return detail::parse_wkt<multi_point>(v);
}

inline auto mline_from_text(std::string_view v)
{
    return detail::parse_wkt<multi_linestring>(v);
}

inline auto mpoly_from_text(std::string_view v)
{
    return detail::parse_wkt<multi_polygon>(v);
}

inline auto geom_coll_from_text(std::string_view v)
{
    return detail::parse_wkt<geometry_collection>(v);
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_GEOM_FROM_TEXT_HPP
