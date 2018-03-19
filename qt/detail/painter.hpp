// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_PAINTER_HPP
#define BARK_QT_DETAIL_PAINTER_HPP

#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QVector>
#include <bark/detail/wkb/visitor.hpp>
#include <bark/qt/common.hpp>
#include <boost/none.hpp>
#include <functional>

namespace bark {
namespace qt {
namespace detail {

class painter {
    const frame& frm_;
    QPainter painter_;
    QVector<QPointF> cached_path_;

    using path = std::reference_wrapper<decltype(cached_path_)>;

public:
    painter(canvas& map, const layer_def& lr)
        : frm_(map.frm), painter_(&map.img)
    {
        check(map);
        painter_.setCompositionMode(QPainter::CompositionMode_Darken);
        painter_.setRenderHint(QPainter::Antialiasing);
        painter_.setPen(lr.pen);
        painter_.setBrush(lr.brush);
    }

    const uint8_t* operator()(const uint8_t* wkb)
    {
        wkb::istream is{wkb};
        wkb::geometry::accept(is, *this);
        return is.data();
    }

    QPointF operator()(double x, double y)
    {
        return forward(frm_, geometry::point{x, y});
    }

    path operator()(uint32_t count, wkb::path)
    {
        cached_path_.clear();
        cached_path_.reserve(count);
        return cached_path_;
    }

    void operator()(path& sum, const QPointF& item, wkb::path)
    {
        sum.get().push_back(item);
    }

    QPainterPath operator()(uint32_t, wkb::chain<wkb::path>) { return {}; }

    void operator()(QPainterPath& sum, const path& item, wkb::chain<wkb::path>)
    {
        sum.addPolygon(item.get());
    }

    template <typename T>
    boost::none_t operator()(uint32_t, T)
    {
        return boost::none;
    }

    template <typename T>
    void operator()(boost::none_t&, const boost::none_t&, T)
    {
    }

    template <uint32_t Code, typename T>
    void operator()(wkb::tagged<Code, T>)
    {
    }

    boost::none_t operator()(const QPointF& res, wkb::point)
    {
        painter_.drawPoint(res);
        return boost::none;
    }

    boost::none_t operator()(const path& res, wkb::linestring)
    {
        painter_.drawPolyline(res.get());
        return boost::none;
    }

    boost::none_t operator()(const QPainterPath& res, wkb::polygon)
    {
        painter_.drawPath(res);
        painter_.fillPath(res, painter_.brush());
        return boost::none;
    }

    template <typename T>
    boost::none_t operator()(const boost::none_t&, T)
    {
        return boost::none;
    }
};

}  // namespace detail
}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_PAINTER_HPP
