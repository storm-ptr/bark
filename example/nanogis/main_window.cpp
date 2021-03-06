// Andrew Naplavkov

#include "main_window.h"
#include "map_widget.h"
#include "sql_widget.h"
#include "task_widget.h"
#include "tree_view.h"
#include "utility.h"
#include <QApplication>
#include <QList>
#include <QMetaType>
#include <QPixmap>
#include <QScreen>
#include <QSplitter>
#include <QStatusBar>
#include <bark/proj/epsg.hpp>
#include <boost/preprocessor/stringize.hpp>

#define META_TYPE(T) qRegisterMetaType<T>(BOOST_PP_STRINGIZE(T))

main_window::main_window()
{
    META_TYPE(bark::qt::layer);
    META_TYPE(QVector<bark::qt::layer>);
    META_TYPE(bark::qt::link);
    META_TYPE(std::string);
    META_TYPE(task_ptr);
    META_TYPE(layer_columns);

    auto splitter = new QSplitter;
    splitter->setOrientation(Qt::Horizontal);

    auto tree = new tree_view(nullptr);
    splitter->addWidget(tree);

    tab_ = new QTabWidget;
    tab_->setTabPosition(QTabWidget::East);
    splitter->addWidget(tab_);

    auto map = new map_widget;
    map_tab_ = tab_->addTab(map, east_icon("map_disabled"), "");

    auto status_bar = new QStatusBar;
    status_bar->setContextMenuPolicy(Qt::CustomContextMenu);

    proj_lbl_ = new QLabel;
    proj_lbl_->setDisabled(true);
    proj_lbl_->setTextFormat(Qt::RichText);
    proj_lbl_->setText(rich_text("map_disabled", "", Qt::AlignLeft));
    status_bar->addWidget(proj_lbl_);

    coords_lbl_ = new QLabel;
    coords_lbl_->setTextFormat(Qt::RichText);
    coords_lbl_->setText(rich_text("globe_disabled", "", Qt::AlignRight));
    status_bar->addPermanentWidget(coords_lbl_);

    setCentralWidget(splitter);
    setStatusBar(status_bar);

    const float w(std::min<float>(width(), height()));
    resize(w, w / 4. * 3.);
    move(QApplication::primaryScreen()->geometry().center() - rect().center());

    QList<int> sizes;
    sizes.push_back(splitter->size().width() * .3);
    sizes.push_back(splitter->size().width() * .7);
    splitter->setSizes(sizes);

    connect(
        tree, &tree_view::attributes_sig, this, &main_window::attributes_slot);
    connect(tree, &tree_view::fit_sig, map, &map_widget::fit_slot);
    connect(tree, &tree_view::show_sig, map, &map_widget::show_slot);
    connect(tree, &tree_view::task_sig, this, &main_window::open_task_slot);
    connect(tree, &tree_view::terminal_sig, this, &main_window::open_sql_slot);
    connect(tree, &tree_view::undistort_sig, map, &map_widget::undistort_slot);

    connect(map, &map_widget::active_sig, this, &main_window::active_map_slot);
    connect(map, &map_widget::idle_sig, this, &main_window::idle_map_slot);
    connect(map,
            &map_widget::coordinates_sig,
            this,
            &main_window::coordinates_slot);
    connect(
        map, &map_widget::projection_sig, this, &main_window::projection_event);

    setWindowIcon(icon("wheel"));
    setWindowTitle("nanogis");
}

void main_window::projection_event(const std::string& pj)
{
    proj_lbl_->setText(rich_text(
        "map",
        limited_text(QString::fromStdString(bark::proj::abbreviation(pj)),
                     Qt::AlignLeft),
        Qt::AlignLeft));
    proj_lbl_->setToolTip(QString::fromStdString(pj));
}

void main_window::coordinates_slot(const QPointF& lon_lat)
{
    if (std::isfinite(lon_lat.x()) && std::isfinite(lon_lat.y()))
        coords_lbl_->setText(rich_text("globe",
                                       QString("%1 %2")
                                           .arg(lon_lat.x(), 0, 'f', 4)
                                           .arg(lon_lat.y(), 0, 'f', 4),
                                       Qt::AlignRight));
    else
        coords_lbl_->setText(rich_text("globe_disabled", "", Qt::AlignRight));
}

void main_window::set_tab_icon(int tab, const QString& resourceIcon)
{
    tab_->setTabIcon(tab, east_icon(resourceIcon));
}

void main_window::active_map_slot()
{
    set_tab_icon(map_tab_, "map");
}

void main_window::idle_map_slot()
{
    set_tab_icon(map_tab_, "map_disabled");
}

void main_window::open_sql_slot(bark::qt::link lnk)
{
    auto wgt = new sql_widget(nullptr, std::move(lnk));
    tab_->addTab(wgt, east_icon("sql"), "");
    tab_->setCurrentWidget(wgt);
    connect(wgt, &sql_widget::close_sig, this, &main_window::tab_remove_slot);
    connect(wgt, &sql_widget::run_sig, this, &main_window::open_task_slot);
}

void main_window::open_task_slot(task_ptr tsk)
{
    auto wgt = new task_widget(nullptr, tsk);
    tab_->addTab(wgt, east_icon("task"), "");
    tab_->setCurrentWidget(wgt);
    connect(wgt, &task_widget::close_sig, this, &main_window::tab_remove_slot);
    connect(wgt, &task_widget::idle_sig, this, &main_window::idle_task_slot);
    wgt->start();
}

void main_window::tab_remove_slot(QWidget* wgt)
{
    tab_->removeTab(tab_->indexOf(wgt));
    wgt->setParent(nullptr);
    wgt->setAttribute(Qt::WA_DeleteOnClose);
    wgt->close();
}

void main_window::idle_task_slot(QWidget* wgt)
{
    set_tab_icon(tab_->indexOf(wgt), "task_disabled");
}

void main_window::attributes_slot(bark::qt::layer lr)
{
    auto ref =
        static_cast<map_widget*>(tab_->widget(map_tab_))->get_georeference();
    auto tsk = std::make_shared<attributes_task>(std::move(lr), std::move(ref));
    open_task_slot(tsk);
}
