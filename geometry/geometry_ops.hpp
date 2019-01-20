// Andrew Naplavkov

#ifndef BARK_GEOMETRY_GEOMETRY_OPS_HPP
#define BARK_GEOMETRY_GEOMETRY_OPS_HPP

#include <algorithm>
#include <array>
#include <bark/geometry/geometry.hpp>
#include <cmath>
#include <stdexcept>

namespace bark::geometry {

inline double left(const box& box)
{
    return std::min<>(box.min_corner().x(), box.max_corner().x());
}

inline double right(const box& box)
{
    return std::max<>(box.min_corner().x(), box.max_corner().x());
}

inline double bottom(const box& box)
{
    return std::min<>(box.min_corner().y(), box.max_corner().y());
}

inline double top(const box& box)
{
    return std::max<>(box.min_corner().y(), box.max_corner().y());
}

inline point left_top(const box& box)
{
    return point{left(box), top(box)};
}

inline point right_bottom(const box& box)
{
    return point{right(box), bottom(box)};
}

inline double width(const box& box)
{
    return right(box) - left(box);
}

inline double height(const box& box)
{
    return top(box) - bottom(box);
}

template <class Size>
double width(const Size& size)
{
    return size.width();
}

template <class Size>
double height(const Size& size)
{
    return size.height();
}

template <class Size>
double max_scale(const Size& size)
{
    return std::max<>(width(size), height(size));
}

template <class LhsSize, class RhsSize>
double max_scale(const LhsSize& lhs, const RhsSize& rhs)
{
    return std::max<>(width(lhs) / width(rhs), height(lhs) / height(rhs));
}

template <class Point>
void check(const Point& point)
{
    if (!std::isfinite(point.x()) || !std::isfinite(point.y()))
        throw std::runtime_error("point error");
}

/// @see "https://en.wikipedia.org/wiki/Quadrant_(plane_geometry)"
inline std::array<box, 4> sub(const box& ext)
{
    auto min = ext.min_corner();
    auto med = boost::geometry::return_centroid<point>(ext);
    auto max = ext.max_corner();
    return {box{med, max},
            box{{min.get<0>(), med.get<1>()}, {med.get<0>(), max.get<1>()}},
            box{min, med},
            box{{med.get<0>(), min.get<1>()}, {max.get<0>(), med.get<1>()}}};
}

inline box pixel(const view& v)
{
    auto min_corner = boost::geometry::return_centroid<point>(v.extent);
    auto max_corner = min_corner;
    boost::geometry::add_value(max_corner, v.scale);
    return {min_corner, max_corner};
}

template <class OStream>
OStream& operator<<(OStream& os, const box& ext)
{
    return os << left(ext) << "," << bottom(ext) << "," << right(ext) << ","
              << top(ext);
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_GEOMETRY_OPS_HPP
