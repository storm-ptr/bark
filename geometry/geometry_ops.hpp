// Andrew Naplavkov

#ifndef BARK_GEOMETRY_GEOMETRY_OPS_HPP
#define BARK_GEOMETRY_GEOMETRY_OPS_HPP

#include <bark/geometry/geometry.hpp>
#include <cmath>
#include <stdexcept>

namespace bark::geometry {

inline double left(const box& val)
{
    return val.min_corner().x();
}

inline double right(const box& val)
{
    return val.max_corner().x();
}

inline double bottom(const box& val)
{
    return val.min_corner().y();
}

inline double top(const box& val)
{
    return val.max_corner().y();
}

inline double width(const box& val)
{
    return right(val) - left(val);
}

inline double height(const box& val)
{
    return top(val) - bottom(val);
}

inline box shift(const box& from, const box& to)
{
    using namespace boost::geometry;
    auto diff = return_centroid<point>(from);
    subtract_point(diff, return_centroid<point>(to));
    auto min_corner = from.min_corner();
    auto max_corner = from.max_corner();
    add_point(min_corner, diff);
    add_point(max_corner, diff);
    return {min_corner, max_corner};
}

template <class Point>
void check(const Point& val)
{
    if (!std::isfinite(val.x()) || !std::isfinite(val.y()))
        throw std::runtime_error("invalid point");
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_GEOMETRY_OPS_HPP
