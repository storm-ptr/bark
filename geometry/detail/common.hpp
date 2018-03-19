// Andrew Naplavkov

#ifndef BARK_GEOMETRY_DETAIL_COMMON_HPP
#define BARK_GEOMETRY_DETAIL_COMMON_HPP

#include <bark/geometry/geometry.hpp>

namespace bark {
namespace geometry {
namespace detail {

inline void push_ring(polygon& poly, const linestring& ring)
{
    if (poly.outer().empty())
        poly.outer().assign(ring.begin(), ring.end());
    else
        poly.inners().emplace_back(ring.begin(), ring.end());
}

}  // namespace detail
}  // namespace geometry
}  // namespace bark

#endif  // BARK_GEOMETRY_DETAIL_COMMON_HPP
