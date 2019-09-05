// Andrew Naplavkov

#ifndef BARK_QT_FRAME_PROJ_HPP
#define BARK_QT_FRAME_PROJ_HPP

#include <bark/geometry/geometry_ops.hpp>
#include <bark/qt/common.hpp>

namespace bark::qt {

inline QPointF forward(const georeference& ref, const geometry::point& pos)
{
    geometry::check(pos);
    return {ref.size.width() / 2. + (pos.x() - ref.center.x()) / ref.scale,
            ref.size.height() / 2. + (ref.center.y() - pos.y()) / ref.scale};
}

inline QRectF forward(const georeference& ref, const geometry::box& ext)
{
    using namespace geometry;
    return {forward(ref, point{left(ext), top(ext)}),
            forward(ref, point{right(ext), bottom(ext)})};
}

inline geometry::point backward(const georeference& ref, const QPointF& pos)
{
    geometry::check(pos);
    return {(pos.x() - ref.size.width() / 2.) * ref.scale + ref.center.x(),
            (ref.size.height() / 2. - pos.y()) * ref.scale + ref.center.y()};
}

inline geometry::box backward(const georeference& ref, const QRectF& rect)
{
    return {backward(ref, rect.bottomLeft()), backward(ref, rect.topRight())};
}

}  // namespace bark::qt

#endif  // BARK_QT_FRAME_PROJ_HPP
