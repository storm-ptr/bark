// Andrew Naplavkov

#ifndef BARK_QT_COMMON_OPS_HPP
#define BARK_QT_COMMON_OPS_HPP

#include <QMargins>
#include <QRectF>
#include <bark/db/provider_ops.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/normalize.hpp>
#include <bark/proj/transformer.hpp>
#include <bark/qt/common.hpp>
#include <bark/qt/detail/adapt.hpp>
#include <boost/range/adaptor/filtered.hpp>

namespace bark::qt {

inline bool queryable(const link& lnk)
{
    return lnk.queryable;
}

inline auto projection(const layer& lr)
{
    return lr.provider->projection(lr.name);
}

inline auto extent(const layer& lr)
{
    return lr.provider->extent(lr.name);
}

inline auto table(const layer& lr)
{
    return lr.provider->table(qualifier(lr.name));
}

inline auto script(const layer& from, const link& to)
{
    return to.provider->script(table(from));
}

inline auto attr_names(const layer& lr)
{
    return db::names(table(lr).columns | boost::adaptors::filtered(db::not_same{
                                             db::column_type::Geometry}));
}

inline auto tile_coverage(const layer& lr,
                          const geometry::box& ext,
                          const geometry::box& px)
{
    return lr.provider->tile_coverage(lr.name, ext, px);
}

inline auto spatial_objects(const layer& lr,
                            const geometry::box& ext,
                            const geometry::box& px)
{
    return lr.provider->spatial_objects(lr.name, ext, px);
}

inline void check(const frame& frm)
{
    proj::normalize(frm.projection);
    geometry::check(frm.center);
    if (frm.scale < 0. || qFuzzyCompare(frm.scale / 2., 0.))
        throw std::runtime_error("invalid scale");
}

inline QPointF forward(const frame& frm, const geometry::point& pos)
{
    geometry::check(pos);
    return {frm.size.width() / 2. + (pos.x() - frm.center.x()) / frm.scale,
            frm.size.height() / 2. + (frm.center.y() - pos.y()) / frm.scale};
}

inline QRectF forward(const frame& frm, const geometry::box& ext)
{
    return {forward(frm, geometry::left_top(ext)),
            forward(frm, geometry::right_bottom(ext))};
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

inline geometry::box pixel(const frame& frm)
{
    auto pos = adapt(frm.center);
    return {pos, {pos.x() + frm.scale, pos.y() + frm.scale}};
}

inline geometry::box extent(const frame& frm)
{
    return backward(frm, QRectF{{}, frm.size});
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
        auto sc = geometry::max_scale(ext, frm.size);
        return std::move(frm) | set_center(pos) | set_scale(sc);
    };
};

inline auto undistort(layer lr)
{
    return [lr = std::move(lr)](frame frm) {
        auto pj = projection(lr);
        auto tf = proj::transformer{frm.projection, pj};
        auto pos = tf.forward(adapt(frm.center));
        auto px = tf.forward(pixel(frm));
        px = lr.provider->undistorted_pixel(lr.name, px);
        auto sc = geometry::max_scale(px);
        return std::move(frm) | set_projection(pj) | set_center(pos) |
               set_scale(sc);
    };
}

}  // namespace bark::qt

#endif  // BARK_QT_COMMON_OPS_HPP
