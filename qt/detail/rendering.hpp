// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_RENDERING_HPP
#define BARK_QT_DETAIL_RENDERING_HPP

#include <QMargins>
#include <QPainter>
#include <bark/db/provider.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/envelope.hpp>
#include <bark/geometry/geom_from_wkb.hpp>
#include <bark/proj/transformer.hpp>
#include <bark/qt/common_ops.hpp>
#include <bark/qt/detail/canvas_ops.hpp>
#include <bark/qt/detail/painter.hpp>
#include <iostream>
#include <stdexcept>

namespace bark {
namespace qt {
namespace detail {

template <typename Geometry>
QVector<canvas> mock_rendering(const layer& lr,
                               const Geometry& geom,
                               const frame& frm)
{
    using namespace geometry;

    auto wkb = as_binary(geom);
    auto tf = proj::transformer{projection(lr), frm.projection};
    if (!tf.is_trivial())
        tf.inplace_forward(wkb);

    auto ext = envelope(geom_from_wkb(wkb.data()));
    QMargins margin{};
    margin += lr.pen.width();
    auto wnd = frm | intersect(ext) | resize(margin);
    if (wnd.size.isEmpty())
        return {};

    auto map = make<canvas>(wnd);
    painter{map, lr}(wkb.data());
    return {map};
}

inline QVector<canvas> geometry_rendering(const layer& lr,
                                          const geometry::box& tl,
                                          const frame& frm)
{
    auto tf = proj::transformer{projection(lr), frm.projection};
    QMargins margin{};
    margin += lr.pen.width();
    auto wnd = frm | intersect(tf.forward(tl)) | resize(margin);
    if (wnd.size.isEmpty())
        return {};

    auto v = tf.backward(view(frm));
    auto rows = spatial_objects(lr, {tl, v.scale});
    if (!tf.is_trivial())
        for (auto&& row : rows)
            tf.inplace_forward(const_cast<uint8_t*>(
                boost::get<bark::blob_view>(row[0]).data()));

    auto map = make<canvas>(wnd);
    painter paint(map, lr);
    for (auto& row : rows)
        paint(boost::get<blob_view>(row[0]).data());
    return {map};
}

inline QVector<canvas> raster_rendering(const layer& lr,
                                        const geometry::box& tl,
                                        const frame& frm)
{
    using namespace geometry;

    QVector<canvas> res;
    auto pj = projection(lr);
    auto tf = proj::transformer{pj, frm.projection};
    auto v = tf.backward(view(frm));
    auto rows = spatial_objects(lr, {tl, v.scale});
    for (auto& row : rows) {
        auto wkb = boost::get<blob_view>(row[0]);
        auto bbox = envelope(poly_from_wkb(wkb.data()));
        auto wnd = frm | intersect(tf.forward(bbox));
        if (wnd.size.isEmpty())
            continue;

        auto blob = boost::get<blob_view>(row[1]);
        canvas map;
        if (!map.img.loadFromData(blob.data(), uint(blob.size())))
            throw std::runtime_error("load image error");
        map.frm = frame{} | set_size(map.img) | set_projection(pj) | fit(bbox);
        if (tf.is_trivial())
            res.push_back(map | copy(wnd));
        else
            res.push_back(map | transform(wnd));
    }
    return res;
}

inline QVector<canvas> rendering(const layer& lr,
                                 const geometry::box& tl,
                                 const frame& frm) try {
    switch (type(*lr.provider, lr.name)) {
        case db::layer_type::Invalid:
            break;
        case db::layer_type::Geometry:
            return geometry_rendering(lr, tl, frm);
        case db::layer_type::Raster:
            return raster_rendering(lr, tl, frm);
    }
    throw std::logic_error("layer type");
}
catch (const db::busy_exception&) {
    throw;
}
catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    auto color = lr.brush.color();
    color.setAlphaF(1.);
    auto mock_lr = lr;
    mock_lr.brush.setColor(color);
    mock_lr.brush.setStyle(Qt::DiagCrossPattern);
    return mock_rendering(mock_lr, tl, frm);
}

}  // namespace detail
}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_RENDERING_HPP
