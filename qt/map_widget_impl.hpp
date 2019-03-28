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
    , frm_{frame{} | set_size(size()) |
           set_projection(proj::epsg().find_proj(4326)) |
           qt::fit({{-180., -90.}, {180., 90.}})}
    , map_{make<canvas>(frm_)}
{
    setMouseTracking(true);
}

template <class Functor>
void map_widget::set_frame(Functor f) try {
    future_frm_ = {};
    if (!future_map_.valid())
        idle_event();
    auto tmp = f();
    check(tmp);
    std::swap(frm_, tmp);
    if (frm_.projection != tmp.projection)
        transform_event(frm_.projection);
    update();
}
catch (const std::exception&) {
}

template <class Functor>
void map_widget::set_future_frame(Functor&& f)
{
    future_frm_ = QtConcurrent::run(std::forward<Functor>(f));
    active_event();
}

inline void map_widget::start_rendering()
{
    using namespace std::chrono;
    if (auto render = render_.lock())
        render->cancel();
    auto render = std::make_shared<rendering_task>(frm_);
    future_map_ = render->get_future();
    render_ = render;
    render->start(layers_);
    timer_.start(duration_cast<milliseconds>(UiTimeout).count(), this);
    active_event();
}

inline void map_widget::fit(frame frm)
{
    set_frame([frm] { return frm; });
}

inline void map_widget::fit(layer lr)
{
    set_future_frame([frm = frm_, lr] {
        auto tf = proj::transformer{projection(lr), frm.projection};
        return frm | qt::fit(tf.forward(extent(lr)));
    });
}

inline void map_widget::show(QVector<layer> lrs)
{
    layers_ = std::move(lrs);
    start_rendering();
}

inline void map_widget::undistort(layer lr)
{
    set_future_frame([frm = frm_, lr] { return frm | qt::undistort(lr); });
}

inline QPointF map_widget::lon_lat(QMouseEvent* event) const try {
    proj::transformer tf(frm_.projection, proj::epsg().find_proj(4326));
    auto ll = tf.forward(backward(frm_, event->pos()));
    if (ll.x() < -180 || ll.x() > 180 || ll.y() < -90 || ll.y() > 90)
        return {NAN, NAN};
    return adapt(ll);
}
catch (const std::exception&) {
    return {NAN, NAN};
}

inline void map_widget::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != timer_.timerId()) {
        QWidget::timerEvent(event);
    }
    else if (future_frm_.resultCount() && future_frm_.isResultReadyAt(0)) {
        decltype(future_frm_) tmp;
        std::swap(tmp, future_frm_);
        set_frame([&] { return tmp.resultAt(0); });
        start_rendering();
    }
    else if (auto render = render_.lock()) {
        auto map = render->get_recent();
        if (!map.img.isNull()) {
            map_ = std::move(map);
            update();
        }
    }
    else if (future_map_.valid()) {
        if (future_frm_.isCanceled())
            idle_event();
        map_ = future_map_.get();
        update();
    }
}

inline void map_widget::paintEvent(QPaintEvent*)
{
    if (size() != frm_.size) {
        set_frame([&] { return frm_ | set_size(this->size()); });
    }
    else if (frm_ == map_.frm) {
        QPainter painter(this);
        painter.drawImage(rect(), map_.img);
    }
    else if (frm_.projection == map_.frm.projection) {
        QPainter painter(this);
        auto factor = map_.frm.scale / frm_.scale;
        painter.save();
        painter.translate(offset(map_.frm, frm_));
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
    press_center_ = frm_.center;
    press_pos_ = event->pos();
}

inline void map_widget::mouseDoubleClickEvent(QMouseEvent* event)
{
    mousePressEvent(event);
}

inline void map_widget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
        set_frame([&] {
            auto pos = forward(frm_, adapt(press_center_));
            auto offset = press_pos_ - event->pos();
            return frm_ | set_center(backward(frm_, pos + offset));
        });
    else
        mouse_move_event(lon_lat(event));
}

inline void map_widget::mouseReleaseEvent(QMouseEvent* event)
{
    mouseMoveEvent(event);
    if (event->button() == Qt::LeftButton)
        start_rendering();
}

inline void map_widget::wheelEvent(QWheelEvent* event)
{
    set_frame([&] {
        auto factor = pow(.8, event->angleDelta().y() / 120.);
        auto focus = backward(frm_, event->pos());
        auto offset = (frm_.center - adapt(focus)) * (factor - 1.);
        return frm_ | set_center(frm_.center + offset) |
               set_scale(frm_.scale * factor);
    });
    start_rendering();
}

inline void map_widget::resizeEvent(QResizeEvent* event)
{
    set_frame([&] { return frm_ | set_size(event->size()); });
    start_rendering();
}

inline void map_widget::leaveEvent(QEvent*)
{
    mouse_move_event({NAN, NAN});
}

inline void map_widget::closeEvent(QCloseEvent*)
{
    if (auto render = render_.lock())
        render->cancel();
}

inline void map_widget::showEvent(QShowEvent* event)
{
    if (!event->spontaneous())
        transform_event(frm_.projection);
}

}  // namespace bark::qt

#endif  // BARK_QT_MAP_WIDGET_IMPL_HPP
