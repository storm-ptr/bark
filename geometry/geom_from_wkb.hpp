// Andrew Naplavkov

#ifndef BARK_GEOMETRY_GEOM_FROM_WKB_HPP
#define BARK_GEOMETRY_GEOM_FROM_WKB_HPP

#include <bark/geometry/detail/istream.hpp>
#include <bark/geometry/geometry.hpp>

namespace bark::geometry {

inline auto geom_from_wkb(blob_view data)
{
    return istream{data}.read<wkb::geometry>();
}

inline auto point_from_wkb(blob_view data)
{
    return istream{data}.read<wkb::point>();
}

inline auto line_from_wkb(blob_view data)
{
    return istream{data}.read<wkb::linestring>();
}

inline auto poly_from_wkb(blob_view data)
{
    return istream{data}.read<wkb::polygon>();
}

inline auto mpoint_from_wkb(blob_view data)
{
    return istream{data}.read<wkb::multi_point>();
}

inline auto mline_from_wkb(blob_view data)
{
    return istream{data}.read<wkb::multi_linestring>();
}

inline auto mpoly_from_wkb(blob_view data)
{
    return istream{data}.read<wkb::multi_polygon>();
}

inline auto geom_coll_from_wkb(blob_view data)
{
    return istream{data}.read<wkb::geometry_collection>();
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_GEOM_FROM_WKB_HPP
