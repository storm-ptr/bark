// Andrew Naplavkov

#ifndef BARK_QT_FRAME_OPS_HPP
#define BARK_QT_FRAME_OPS_HPP

#include <QMargins>
#include <QRectF>
#include <bark/db/provider.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/normalize.hpp>
#include <bark/proj/transformer.hpp>
#include <bark/qt/common.hpp>
#include <bark/qt/detail/adapt.hpp>

namespace bark::qt {

inline void check(const frame& frm)
{
    proj::normalize(frm.projection);
    geometry::check(frm.center);
    if (frm.scale < 0. || qFuzzyCompare(frm.scale / 2., 0.))
        throw std::runtime_error("invalid scale");
}

inline bool operator==(const frame& lhs, const frame& rhs)
{
    return lhs.projection == rhs.projection && lhs.center == rhs.center &&
           qFuzzyCompare(lhs.scale, rhs.scale) && lhs.size == rhs.size;
}

inline QPointF offset(const frame& lhs, const frame& rhs)
{
    return forward(rhs, backward(lhs, QPointF{}));
}

template <class Functor>
frame operator|(frame frm, Functor f)
{
    return f(std::move(frm));
}

template <class Size>
auto set_size(Size sz)
{
    return [sz = std::move(sz)](frame frm) {
        frm.size = {sz.width(), sz.height()};
        return frm;
    };
}

inline auto set_projection(std::string pj)
{
    return [pj = std::move(pj)](frame frm) {
        frm.projection = pj;
        return frm;
    };
}

template <class Point>
auto set_center(Point pos)
{
    return [pos = std::move(pos)](frame frm) {
        frm.center = {pos.x(), pos.y()};
        return frm;
    };
}

inline auto set_scale(qreal sc)
{
    return [=](frame frm) {
        frm.scale = sc;
        return frm;
    };
}

inline auto resize(QMargins mar)
{
    return [mar = std::move(mar)](frame frm) {
        auto sz = QRect{{}, frm.size}.marginsAdded(mar);
        return std::move(frm) | set_size(sz);
    };
}

inline auto intersect(geometry::box ext)
{
    return [ext = std::move(ext)](frame frm) {
        auto rect = forward(frm, ext).intersected({{}, frm.size});
        auto pos = backward(frm, rect.center());
        auto sz = rect.toAlignedRect().size();
        return std::move(frm) | set_center(pos) | set_size(sz);
    };
}

inline auto fit(geometry::box ext)
{
    return [ext = std::move(ext)](frame frm) {
        auto pos = boost::geometry::return_centroid<geometry::point>(ext);
        auto sc = std::max<>(geometry::width(ext) / frm.size.width(),
                             geometry::height(ext) / frm.size.height());
        return std::move(frm) | set_center(pos) | set_scale(sc);
    };
};

inline auto undistort(layer lr)
{
    return [lr = std::move(lr)](frame frm) {
        auto pj = lr.provider->projection(lr.name);
        auto tf = proj::transformer{frm.projection, pj};
        auto pos = tf.forward(adapt(frm.center));
        auto px = tf.forward(pixel(frm));
        px = lr.provider->undistorted_pixel(lr.name, px);
        auto sc = std::max<>(geometry::width(px), geometry::height(px));
        return std::move(frm) | set_projection(pj) | set_center(pos) |
               set_scale(sc);
    };
}

}  // namespace bark::qt

#endif  // BARK_QT_FRAME_OPS_HPP
