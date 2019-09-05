// Andrew Naplavkov

#ifndef BARK_QT_CANVAS_OPS_HPP
#define BARK_QT_CANVAS_OPS_HPP

#include <QPainter>
#include <bark/detail/grid.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/transformer.hpp>
#include <bark/qt/detail/geoimage.hpp>
#include <bark/qt/detail/georeference_ops.hpp>
#include <stdexcept>

namespace bark::qt {

inline void check(const geoimage& map)
{
    check(map.ref);
    if (map.ref.size != QSize{map.img.width(), map.img.height()})
        throw std::logic_error("size mismatch");
}

template <class T>
T make(const georeference&);

template <>
inline QImage make<QImage>(const georeference& ref)
{
    QImage res{ref.size, QImage::Format_ARGB32_Premultiplied};
    res.fill(Qt::transparent);
    return res;
}

template <>
inline geoimage make<geoimage>(const georeference& ref)
{
    return {ref, make<QImage>(ref)};
}

template <class Functor>
geoimage operator|(const geoimage& map, Functor f)
{
    return f(map);
}

inline auto copy(const georeference& ref)
{
    return [=](const geoimage& map) {
        auto res = make<geoimage>(ref);
        QPainter painter{&res.img};
        auto rect = forward(res.ref, extent(map.ref)).toAlignedRect();
        painter.drawImage(rect, map.img);
        return res;
    };
}

inline auto transform(const georeference& ref)
{
    return [=](const geoimage& map) {
        auto res = make<geoimage>(ref);

        auto gr = grid(boost::geometry::return_buffer<geometry::box>(
                           extent(ref), -ref.scale / 2),
                       ref.size.height(),
                       ref.size.width());
        auto tf = proj::transformer{map.ref.projection, ref.projection};
        tf.inplace_backward(gr.begin(), gr.end());
        auto ext = extent(map.ref);
        auto left = geometry::left(ext);
        auto top = geometry::top(ext);
        auto ratio_x = map.img.width() / geometry::width(ext);
        auto ratio_y = map.img.height() / geometry::height(ext);

        for (size_t row = 0; row < gr.rows(); ++row)
            for (size_t col = 0; col < gr.cols(); ++col) {
                auto x = gr.x(row, col);
                auto y = gr.y(row, col);
                QPoint from((x - left) * ratio_x, (top - y) * ratio_y);
                if (!map.img.valid(from))
                    continue;
                QPoint to(int(col), int(gr.rows() - 1 - row));
                res.img.setPixel(to, map.img.pixel(from));
            }
        return res;
    };
}

}  // namespace bark::qt

#endif  // BARK_QT_CANVAS_OPS_HPP
