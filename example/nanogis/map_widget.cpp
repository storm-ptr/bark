#include "map_widget.h"
#include <bark/qt/map_widget_impl.hpp>

map_widget::map_widget(QWidget* parent) : bark::qt::map_widget(parent) {}

void map_widget::fit_slot(bark::qt::layer lr)
{
    fit(std::move(lr));
}

void map_widget::show_slot(QVector<bark::qt::layer> lrs)
{
    show(std::move(lrs));
}

void map_widget::undistort_slot(bark::qt::layer lr)
{
    undistort(std::move(lr));
}

void map_widget::active_event()
{
    emit active_sig();
}

void map_widget::idle_event()
{
    emit idle_sig();
}

void map_widget::mouse_move_event(QPointF lon_lat)
{
    emit mouse_move_sig(lon_lat);
}

void map_widget::transform_event(std::string pj)
{
    emit transform_sig(std::move(pj));
}
