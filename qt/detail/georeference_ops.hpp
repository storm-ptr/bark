// Andrew Naplavkov

#ifndef BARK_QT_FRAME_OPS_HPP
#define BARK_QT_FRAME_OPS_HPP

#include <QMargins>
#include <QRectF>
#include <bark/db/provider.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/epsg.hpp>
#include <bark/proj/transformer.hpp>
#include <bark/qt/common.hpp>
#include <bark/qt/detail/adapt.hpp>

namespace bark::qt {

inline void check(const georeference& ref)
{
    geometry::check(ref.center);
    if (ref.scale < 0. || qFuzzyCompare(ref.scale / 2., 0.))
        throw std::runtime_error("invalid scale");
}

inline bool operator==(const georeference& lhs, const georeference& rhs)
{
    return lhs.projection == rhs.projection && lhs.center == rhs.center &&
           qFuzzyCompare(lhs.scale, rhs.scale) && lhs.size == rhs.size;
}

inline QPointF offset(const georeference& lhs, const georeference& rhs)
{
    return forward(rhs, backward(lhs, QPointF{}));
}

template <class Functor>
georeference operator|(georeference ref, Functor f)
{
    return f(std::move(ref));
}

template <class Size>
auto set_size(Size sz)
{
    return [sz = std::move(sz)](georeference ref) {
        ref.size = {sz.width(), sz.height()};
        return ref;
    };
}

inline auto set_projection(std::string pj)
{
    return [pj = std::move(pj)](georeference ref) {
        ref.projection = pj;
        return ref;
    };
}

template <class Point>
auto set_center(Point pos)
{
    return [pos = std::move(pos)](georeference ref) {
        ref.center = {pos.x(), pos.y()};
        return ref;
    };
}

inline auto set_scale(qreal sc)
{
    return [sc](georeference ref) {
        ref.scale = sc;
        return ref;
    };
}

inline auto resize(QMargins mar)
{
    return [mar = std::move(mar)](georeference ref) {
        auto sz = QRect{{}, ref.size}.marginsAdded(mar);
        return std::move(ref) | set_size(sz);
    };
}

inline auto intersect(geometry::box ext)
{
    return [ext = std::move(ext)](georeference ref) {
        auto rect = forward(ref, ext).intersected({{}, ref.size});
        auto pos = backward(ref, rect.center());
        auto sz = rect.toAlignedRect().size();
        return std::move(ref) | set_center(pos) | set_size(sz);
    };
}

inline auto fit(geometry::box ext)
{
    return [ext = std::move(ext)](georeference ref) {
        auto pos = boost::geometry::return_centroid<geometry::point>(ext);
        auto sc = std::max<>(geometry::width(ext) / ref.size.width(),
                             geometry::height(ext) / ref.size.height());
        return std::move(ref) | set_center(pos) | set_scale(sc);
    };
};

template <class Point>
auto set_general_perspective(Point lon_lat)
{
    return [lon_lat = std::move(lon_lat)](georeference ref) {
        std::ostringstream os;
        os << "+proj=ortho +lat_0=" << lon_lat.y() << " +lon_0=" << lon_lat.x()
           << " +x_0=0 +y_0=0 +a=6370997 +b=6370997 +units=m +no_defs";
        auto pj = os.str();
        auto tf = proj::transformer{ref.projection, pj};
        auto ext = tf.forward(extent(ref));
        return std::move(ref) | set_projection(pj) | fit(ext);
    };
}

inline auto undistort(layer lr)
{
    return [lr = std::move(lr)](georeference ref) {
        auto pj = lr.provider->projection(lr.name);
        auto tf = proj::transformer{ref.projection, pj};
        auto pos = tf.forward(adapt(ref.center));
        auto px = tf.forward(pixel(ref));
        px = lr.provider->undistorted_pixel(lr.name, px);
        auto sc = std::max<>(geometry::width(px), geometry::height(px));
        return std::move(ref) | set_projection(pj) | set_center(pos) |
               set_scale(sc);
    };
}

template <class Size>
auto make_globe_mercator(Size sz)
{
    auto longlat = proj::epsg().find_proj(4326);
    auto merc = proj::epsg().find_proj(3857);
    auto tf = proj::transformer{longlat, merc};
    auto ext = tf.forward({{-180., -90.}, {180., 90.}});
    return georeference{} | set_size(sz) | set_projection(merc) | fit(ext);
}

}  // namespace bark::qt

#endif  // BARK_QT_FRAME_OPS_HPP
