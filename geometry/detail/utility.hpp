// Andrew Naplavkov

#ifndef BARK_GEOMETRY_DETAIL_UTILITY_HPP
#define BARK_GEOMETRY_DETAIL_UTILITY_HPP

#include <bark/geometry/geometry.hpp>

namespace bark::geometry::detail {

inline void push_ring(polygon& poly, const linestring& ring)
{
    if (poly.outer().empty())
        poly.outer().assign(ring.begin(), ring.end());
    else
        poly.inners().emplace_back(ring.begin(), ring.end());
}

}  // namespace bark::geometry::detail

#endif  // BARK_GEOMETRY_DETAIL_UTILITY_HPP
