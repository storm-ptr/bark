// Andrew Naplavkov
// include in *.cpp files only (Qt MOC workaround)

#ifndef BARK_QT_MAP_WIDGET_IMPL_HPP
#define BARK_QT_MAP_WIDGET_IMPL_HPP

#include <QPainter>
#include <QtConcurrent/QtConcurrentRun>
#include <bark/detail/utility.hpp>
#include <bark/proj/epsg.hpp>
#include <bark/qt/common_ops.hpp>
#include <bark/qt/detail/rendering_task.hpp>
#include <bark/qt/map_widget.hpp>
#include <exception>

namespace bark::qt {

inline map_widget::map_widget(QWidget* parent)
    : QWidget(parent)
    , ref_{georeference{} | set_size(size()) |
           set_projection(proj::epsg().find_proj(4326)) |
           qt::fit({{-180., -90.}, {180., 90.}})}
    , map_{make<geoimage>(ref_)}
{
    setMouseTracking(true);
}

template <class Functor>
void map_widget::set_georeference(Functor f)
try {
    future_ref_ = {};
    if (!future_map_.valid())
        idle_event();
    auto tmp = f();
    check(tmp);
    std::swap(ref_, tmp);
    if (ref_.projection != tmp.projection)
        projection_event(ref_.projection);
    update();
}
catch (const std::exception&) {
}

template <class Functor>
void map_widget::set_future_georeference(Functor&& f)
{
    future_ref_ = QtConcurrent::run(std::forward<Functor>(f));
    active_event();
}

inline void map_widget::start_rendering()
{
    using namespace std::chrono;
    if (auto render = render_.lock())
        render->cancel();
    auto render = std::make_shared<rendering_task>(ref_);
    future_map_ = render->get_future();
    render_ = render;
    render->start(layers_);
    timer_.start(duration_cast<milliseconds>(UiTimeout).count(), this);
    active_event();
}

inline void map_widget::fit(georeference ref)
{
    set_georeference([ref = std::move(ref)] { return ref; });
}

inline void map_widget::fit(layer lr)
{
    set_future_georeference([ref = ref_, lr = std::move(lr)] {
        auto tf = proj::transformer{projection(lr), ref.projection};
        return ref | qt::fit(tf.forward(extent(lr)));
    });
}

inline void map_widget::show(QVector<layer> lrs)
{
    layers_ = std::move(lrs);
    start_rendering();
}

inline void map_widget::undistort(layer lr)
{
    set_future_georeference(
        [ref = ref_, lr = std::move(lr)] { return ref | qt::undistort(lr); });
}

inline QPointF map_widget::lon_lat(QMouseEvent* event) const
try {
    proj::transformer tf(ref_.projection, proj::epsg().find_proj(4326));
    auto ll = tf.forward(backward(ref_, event->pos()));
    if (ll.x() < -180 || ll.x() > 180 || ll.y() < -90 || ll.y() > 90)
        return {NAN, NAN};
    return adapt(ll);
}
catch (const std::exception&) {
    return {NAN, NAN};
}

inline void map_widget::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != timer_.timerId())
        QWidget::timerEvent(event);
    else if (future_ref_.resultCount() && future_ref_.isResultReadyAt(0)) {
        decltype(future_ref_) tmp;
        std::swap(tmp, future_ref_);
        set_georeference([&] { return tmp.resultAt(0); });
        start_rendering();
    }
    else if (auto render = render_.lock()) {
        if (auto map = render->get_recent(); !map.img.isNull()) {
            map_ = std::move(map);
            update();
        }
    }
    else if (future_map_.valid()) {
        if (future_ref_.isCanceled())
            idle_event();
        map_ = future_map_.get();
        update();
    }
}

inline void map_widget::paintEvent(QPaintEvent*)
{
    if (size() != ref_.size)
        set_georeference([this] { return ref_ | set_size(this->size()); });
    else if (ref_ == map_.ref) {
        QPainter painter(this);
        painter.drawImage(rect(), map_.img);
    }
    else if (ref_.projection == map_.ref.projection) {
        QPainter painter(this);
        auto factor = map_.ref.scale / ref_.scale;
        painter.save();
        painter.translate(offset(map_.ref, ref_));
        painter.scale(factor, factor);
        QRectF exposed =
            painter.matrix().inverted().mapRect(rect()).adjusted(-1, -1, 1, 1);
        painter.drawImage(exposed, map_.img, exposed);
        painter.restore();
    }
}

inline void map_widget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;
    press_center_ = ref_.center;
    press_pos_ = event->pos();
}

inline void map_widget::mouseDoubleClickEvent(QMouseEvent* event)
{
    mousePressEvent(event);
}

inline void map_widget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
        set_georeference([&] {
            auto pos = forward(ref_, adapt(press_center_));
            auto offset = press_pos_ - event->pos();
            return ref_ | set_center(backward(ref_, pos + offset));
        });
    else
        coordinates_event(lon_lat(event));
}

inline void map_widget::mouseReleaseEvent(QMouseEvent* event)
{
    mouseMoveEvent(event);
    if (event->button() == Qt::LeftButton)
        start_rendering();
}

inline void map_widget::wheelEvent(QWheelEvent* event)
{
    set_georeference([&] {
        auto factor = pow(.8, event->angleDelta().y() / 120.);
        auto focus = backward(ref_, event->pos());
        auto offset = (ref_.center - adapt(focus)) * (factor - 1.);
        return ref_ | set_center(ref_.center + offset) |
               set_scale(ref_.scale * factor);
    });
    start_rendering();
}

inline void map_widget::resizeEvent(QResizeEvent* event)
{
    set_georeference([&] { return ref_ | set_size(event->size()); });
    start_rendering();
}

inline void map_widget::leaveEvent(QEvent*)
{
    coordinates_event({NAN, NAN});
}

inline void map_widget::closeEvent(QCloseEvent*)
{
    if (auto render = render_.lock())
        render->cancel();
}

inline void map_widget::showEvent(QShowEvent* event)
{
    if (!event->spontaneous())
        projection_event(ref_.projection);
}

}  // namespace bark::qt

#endif  // BARK_QT_MAP_WIDGET_IMPL_HPP
