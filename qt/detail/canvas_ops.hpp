// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_CANVAS_OPS_HPP
#define BARK_QT_DETAIL_CANVAS_OPS_HPP

#include <QPainter>
#include <bark/detail/grid.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/transformer.hpp>
#include <bark/qt/common_ops.hpp>
#include <bark/qt/detail/canvas.hpp>
#include <stdexcept>

namespace bark {
namespace qt {
namespace detail {

inline void check(const canvas& map)
{
    check(map.frm);
    if (map.frm.size != QSize{map.img.width(), map.img.height()})
        throw std::logic_error("size mismatch");
}

template <typename T>
T make(const frame&);

template <>
inline QImage make<QImage>(const frame& frm)
{
    QImage res{frm.size, QImage::Format_ARGB32_Premultiplied};
    res.fill(Qt::transparent);
    return res;
}

template <>
inline canvas make<canvas>(const frame& frm)
{
    return {frm, make<QImage>(frm)};
}

template <typename Functor>
canvas operator|(const canvas& map, Functor fn)
{
    return fn(map);
}

inline auto copy(const frame& frm)
{
    return [=](const canvas& map) {
        auto res = make<canvas>(frm);
        QPainter painter{&res.img};
        auto rect = forward(res.frm, extent(map.frm)).toAlignedRect();
        painter.drawImage(rect, map.img);
        return res;
    };
}

inline auto transform(const frame& frm)
{
    return [=](const canvas& map) {
        auto res = make<canvas>(frm);

        grid gr(boost::geometry::return_buffer<geometry::box>(extent(frm),
                                                              -frm.scale / 2),
                frm.size.height(),
                frm.size.width());
        auto tf = proj::transformer{map.frm.projection, frm.projection};
        tf.inplace_backward(gr.begin(), gr.end());
        auto ext = extent(map.frm);
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

}  // namespace detail
}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_CANVAS_OPS_HPP
