// Andrew Naplavkov

#ifndef BARK_QT_RENDERING_HPP
#define BARK_QT_RENDERING_HPP

#include <QMargins>
#include <QPainter>
#include <bark/db/provider.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/envelope.hpp>
#include <bark/geometry/geom_from_wkb.hpp>
#include <bark/proj/transformer.hpp>
#include <bark/qt/common_ops.hpp>
#include <bark/qt/detail/geoimage_ops.hpp>
#include <bark/qt/detail/painter.hpp>
#include <iostream>
#include <stdexcept>

namespace bark::qt {

template <class Geometry>
QVector<geoimage> mock_rendering(const layer& lr,
                                 const Geometry& geom,
                                 const georeference& ref)
{
    using namespace geometry;

    auto wkb = as_binary(geom);
    auto tf = proj::transformer{projection(lr), ref.projection};
    if (!tf.is_trivial())
        tf.inplace_forward(wkb);

    auto ext = envelope(geom_from_wkb(wkb));
    QMargins margin{};
    margin += lr.pen.width();
    auto wnd = ref | intersect(ext) | resize(margin);
    if (wnd.size.isEmpty())
        return {};

    auto map = make<geoimage>(wnd);
    painter{map, lr}(wkb);
    return {map};
}

inline QVector<geoimage> geometry_rendering(const layer& lr,
                                            const geometry::box& tl,
                                            const georeference& ref)
{
    auto tf = proj::transformer{projection(lr), ref.projection};
    QMargins margin{};
    margin += lr.pen.width();
    auto wnd = ref | intersect(tf.forward(tl)) | resize(margin);
    if (wnd.size.isEmpty())
        return {};

    auto px = tf.backward(pixel(ref));
    auto objects = spatial_objects(lr, tl, px);
    auto rows = select(objects);
    if (!tf.is_trivial())
        db::for_each_blob(rows, 0, tf.inplace_forward());

    auto map = make<geoimage>(wnd);
    db::for_each_blob(rows, 0, painter{map, lr});
    return {map};
}

inline QVector<geoimage> raster_rendering(const layer& lr,
                                          const geometry::box& tl,
                                          const georeference& ref)
{
    using namespace geometry;

    QVector<geoimage> res;
    auto pj = projection(lr);
    auto tf = proj::transformer{pj, ref.projection};
    auto px = tf.backward(pixel(ref));
    auto objects = spatial_objects(lr, tl, px);
    for (auto& row : select(objects)) {
        auto wkb = std::get<blob_view>(row[0]);
        auto bbox = envelope(poly_from_wkb(wkb));
        auto wnd = ref | intersect(tf.forward(bbox));
        if (wnd.size.isEmpty())
            continue;

        auto img = std::get<blob_view>(row[1]);
        geoimage map;
        if (!map.img.loadFromData((uchar*)img.data(), (int)img.size()))
            throw std::runtime_error("load image error");
        map.ref =
            georeference{} | set_size(map.img) | set_projection(pj) | fit(bbox);
        if (tf.is_trivial())
            res.push_back(map | copy(wnd));
        else
            res.push_back(map | transform(wnd));
    }
    return res;
}

inline QVector<geoimage> rendering(const layer& lr,
                                   const geometry::box& tl,
                                   const georeference& ref) try {
    switch (lr.provider->dir().at(lr.name)) {
        case db::layer_type::Invalid:
            break;
        case db::layer_type::Geometry:
            return geometry_rendering(lr, tl, ref);
        case db::layer_type::Raster:
            return raster_rendering(lr, tl, ref);
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
    return mock_rendering(mock_lr, tl, ref);
}

}  // namespace bark::qt

#endif  // BARK_QT_RENDERING_HPP
