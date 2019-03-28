// Andrew Naplavkov

#ifndef BARK_GEOMETRY_UTILITY_HPP
#define BARK_GEOMETRY_UTILITY_HPP

#include <bark/geometry/geometry.hpp>

namespace bark::geometry {

inline void push_ring(polygon& poly, const linestring& ring)
{
    if (poly.outer().empty())
        poly.outer().assign(ring.begin(), ring.end());
    else
        poly.inners().emplace_back(ring.begin(), ring.end());
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_UTILITY_HPP
