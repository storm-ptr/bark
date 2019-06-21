// Andrew Naplavkov

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "task.h"
#include <QLabel>
#include <QMainWindow>
#include <QPointF>
#include <QString>
#include <QTabWidget>

class main_window : public QMainWindow {
    Q_OBJECT

public:
    main_window();

private slots:
    void active_map_slot();
    void attributes_slot(bark::qt::layer);
    void coordinates_slot(const QPointF& lon_lat);
    void idle_map_slot();
    void idle_task_slot(QWidget*);
    void open_sql_slot(bark::qt::link);
    void open_task_slot(task_ptr);
    void projection_event(const std::string& pj);
    void tab_remove_slot(QWidget*);

private:
    QTabWidget* tab_;
    int map_tab_;
    QLabel* proj_lbl_;
    QLabel* coords_lbl_;

    void set_tab_icon(int tab, const QString& resourceIcon);
};

#endif  // MAIN_WINDOW_H
