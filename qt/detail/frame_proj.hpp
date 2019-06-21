// Andrew Naplavkov

#ifndef BARK_QT_FRAME_PROJ_HPP
#define BARK_QT_FRAME_PROJ_HPP

#include <bark/geometry/geometry_ops.hpp>
#include <bark/qt/common.hpp>

namespace bark::qt {

inline QPointF forward(const frame& frm, const geometry::point& pos)
{
    geometry::check(pos);
    return {frm.size.width() / 2. + (pos.x() - frm.center.x()) / frm.scale,
            frm.size.height() / 2. + (frm.center.y() - pos.y()) / frm.scale};
}

inline QRectF forward(const frame& frm, const geometry::box& ext)
{
    using namespace geometry;
    return {forward(frm, point{left(ext), top(ext)}),
            forward(frm, point{right(ext), bottom(ext)})};
}

inline geometry::point backward(const frame& frm, const QPointF& pos)
{
    geometry::check(pos);
    return {(pos.x() - frm.size.width() / 2.) * frm.scale + frm.center.x(),
            (frm.size.height() / 2. - pos.y()) * frm.scale + frm.center.y()};
}

inline geometry::box backward(const frame& frm, const QRectF& rect)
{
    return {backward(frm, rect.bottomLeft()), backward(frm, rect.topRight())};
}

}  // namespace bark::qt

#endif  // BARK_QT_FRAME_PROJ_HPP
