// Andrew Naplavkov

#ifndef BARK_QT_MAP_WIDGET_HPP
#define BARK_QT_MAP_WIDGET_HPP

#include <QBasicTimer>
#include <QFuture>
#include <QMouseEvent>
#include <QPointF>
#include <QString>
#include <QTimerEvent>
#include <QVector>
#include <QWidget>
#include <bark/qt/common.hpp>
#include <bark/qt/detail/canvas.hpp>
#include <future>
#include <memory>
#include <string>

namespace bark {
namespace qt {
namespace detail {

class rendering_task;

}  // namespace detail

/**
 * The class receives events from the window system and represent a cartography
 * interface on the screen.
 */
class map_widget : public QWidget {
public:
    explicit map_widget(QWidget* parent);

    const frame& get_frame() const { return frm_; }

    void fit(frame);
    void fit(layer);
    void show(QVector<layer>);
    void undistort(layer);

protected:
    virtual void active_event() {}
    virtual void idle_event() {}
    virtual void mouse_move_event(QPointF /*lon_lat*/) {}
    virtual void transform_event(std::string /*pj*/) {}

    void closeEvent(QCloseEvent*) override;
    void leaveEvent(QEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
    void timerEvent(QTimerEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    frame frm_;
    QFuture<frame> future_frm_;
    detail::canvas map_;
    std::future<detail::canvas> future_map_;
    QVector<layer> layers_;
    std::weak_ptr<detail::rendering_task> render_;
    QPointF press_center_;
    QPoint press_pos_;
    QBasicTimer timer_;

    void start_rendering();
    QPointF lon_lat(QMouseEvent*) const;

    template <typename Functor>
    void set_frame(Functor);

    template <typename Functor>
    void set_future_frame(Functor&&);
};

}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_MAP_WIDGET_HPP
