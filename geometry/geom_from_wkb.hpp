// Andrew Naplavkov

#ifndef BARK_GEOMETRY_GEOM_FROM_WKB_HPP
#define BARK_GEOMETRY_GEOM_FROM_WKB_HPP

#include <bark/geometry/detail/istream.hpp>
#include <bark/geometry/geometry.hpp>

namespace bark {
namespace geometry {

inline auto geom_from_wkb(const uint8_t* ptr)
{
    return detail::istream{ptr}.read<wkb::geometry>();
}

inline auto point_from_wkb(const uint8_t* ptr)
{
    return detail::istream{ptr}.read<wkb::point>();
}

inline auto line_from_wkb(const uint8_t* ptr)
{
    return detail::istream{ptr}.read<wkb::linestring>();
}

inline auto poly_from_wkb(const uint8_t* ptr)
{
    return detail::istream{ptr}.read<wkb::polygon>();
}

inline auto mpoint_from_wkb(const uint8_t* ptr)
{
    return detail::istream{ptr}.read<wkb::multi_point>();
}

inline auto mline_from_wkb(const uint8_t* ptr)
{
    return detail::istream{ptr}.read<wkb::multi_linestring>();
}

inline auto mpoly_from_wkb(const uint8_t* ptr)
{
    return detail::istream{ptr}.read<wkb::multi_polygon>();
}

inline auto geom_coll_from_wkb(const uint8_t* ptr)
{
    return detail::istream{ptr}.read<wkb::geometry_collection>();
}

}  // namespace geometry
}  // namespace bark

#endif  // BARK_GEOMETRY_GEOM_FROM_WKB_HPP
