#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

#include <QPointF>
#include <QString>
#include <bark/qt/map_widget.hpp>

class renderer;

class map_widget : public bark::qt::map_widget {
    Q_OBJECT

public:
    explicit map_widget(QWidget* parent = nullptr);

public slots:
    void fit_slot(bark::qt::layer);
    void show_slot(QVector<bark::qt::layer>);
    void undistort_slot(bark::qt::layer);

signals:
    void active_sig();
    void idle_sig();
    void mouse_move_sig(QPointF lon_lat);
    void transform_sig(std::string pj);

protected:
    void active_event() override;
    void idle_event() override;
    void mouse_move_event(QPointF lon_lat) override;
    void transform_event(std::string pj) override;
};

#endif  // MAP_WIDGET_H
