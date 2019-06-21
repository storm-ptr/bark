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

namespace bark::qt {

class rendering_task;

/// The cartography interface on the screen (panning, zooming)
class map_widget : public QWidget {
public:
    explicit map_widget(QWidget* parent);

    /// Returns drawing space
    const frame& get_frame() const { return frm_; }

    /// Zooms to fit drawing space
    void fit(frame);

    /// Zooms to fit data set
    void fit(layer);

    /// Draws data sets
    void show(QVector<layer>);

    /// Changes projection and scale to avoid distortion
    void undistort(layer);

protected:
    /// Occurs when the map starts to draw
    virtual void active_event() {}

    /// Occurs when the map finishes to draw
    virtual void idle_event() {}

    /// Occurs when the mouse cursor is moved
    virtual void coordinates_event(QPointF /*lon_lat*/) {}

    /// Occurs when the spatial reference system is changed
    virtual void projection_event(std::string /*pj*/) {}

    // override
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
    canvas map_;
    std::future<canvas> future_map_;
    QVector<layer> layers_;
    std::weak_ptr<rendering_task> render_;
    QPointF press_center_;
    QPoint press_pos_;
    QBasicTimer timer_;

    void start_rendering();
    QPointF lon_lat(QMouseEvent*) const;

    template <class Functor>
    void set_frame(Functor);

    template <class Functor>
    void set_future_frame(Functor&&);
};

}  // namespace bark::qt

#endif  // BARK_QT_MAP_WIDGET_HPP
