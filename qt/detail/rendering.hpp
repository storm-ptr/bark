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

namespace bark::qt::detail {

template <class Geometry>
QVector<canvas> mock_rendering(const layer& lr,
                               const Geometry& geom,
                               const frame& frm)
{
    using namespace geometry;

    auto wkb = as_binary(geom);
    auto tf = proj::transformer{projection(lr), frm.projection};
    if (!tf.is_trivial())
        tf.inplace_forward(wkb);

    auto ext = envelope(geom_from_wkb(wkb));
    QMargins margin{};
    margin += lr.pen.width();
    auto wnd = frm | intersect(ext) | resize(margin);
    if (wnd.size.isEmpty())
        return {};

    auto map = make<canvas>(wnd);
    painter{map, lr}(wkb);
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

    auto px = tf.backward(pixel(frm));
    auto rows = spatial_objects(lr, tl, px);
    auto rng = range(rows);
    if (!tf.is_trivial())
        db::for_each_blob(rng, 0, tf.inplace_forward());

    auto map = make<canvas>(wnd);
    db::for_each_blob(rng, 0, painter{map, lr});
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
    auto px = tf.backward(pixel(frm));
    auto rows = spatial_objects(lr, tl, px);
    for (auto& row : range(rows)) {
        auto wkb = std::get<blob_view>(row[0]);
        auto bbox = envelope(poly_from_wkb(wkb));
        auto wnd = frm | intersect(tf.forward(bbox));
        if (wnd.size.isEmpty())
            continue;

        auto img = std::get<blob_view>(row[1]);
        canvas map;
        if (!map.img.loadFromData((uchar*)img.data(), (int)img.size()))
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

}  // namespace bark::qt::detail

#endif  // BARK_QT_DETAIL_RENDERING_HPP
