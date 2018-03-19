// Andrew Naplavkov

#include "tree_view.h"
#include "column_matching_dialog.h"
#include "insertion_task.h"
#include "tools.h"
#include <QColorDialog>
#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <bark/qt/tree_model_impl.hpp>

#define ACTION(action, icon_id, text)                       \
    action##_act_ = new QAction(icon(icon_id), text, this); \
    action##_act_->setIconVisibleInMenu(true);              \
    connect(action##_act_, &QAction::triggered, this, &tree_view::action##_slot)

tree_view::tree_view(QWidget* parent) : QTreeView(parent), model_(nullptr)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setHeaderHidden(true);
    setModel(&model_);
    connect(&model_,
            &QAbstractItemModel::rowsInserted,
            this,
            &tree_view::update_slot);
    connect(&model_,
            &QAbstractItemModel::rowsRemoved,
            this,
            &tree_view::update_slot);
    connect(&model_,
            &QAbstractItemModel::dataChanged,
            this,
            &tree_view::update_slot);
    connect(this,
            &QTreeView::customContextMenuRequested,
            this,
            &tree_view::menu_slot);
    ACTION(attach_files, QStyle::SP_FileDialogNewFolder, "attach file(s)");
    ACTION(attach_uri, QStyle::SP_FileDialogNewFolder, "attach URI");
    ACTION(attributes, "sql", "attributes (limited)");
    ACTION(brush_color, "palette", "filling color");
    ACTION(copy, "copy", "copy this layer");
    ACTION(copy_checked, "copy", "copy checked layer(s)");
    ACTION(create, QStyle::SP_FileIcon, "new project");
    ACTION(del, QStyle::SP_DialogDiscardButton, "delete this layer");
    ACTION(detach, QStyle::SP_DialogCloseButton, "detach");
    ACTION(fit, "zoom", "zoom to fit");
    ACTION(metadata, QStyle::SP_FileDialogInfoView, "metadata");
    ACTION(paste, "paste", "paste layer(s)");
    ACTION(pen_color, "palette", "outline color");
    ACTION(pen_width, "palette", "outline width");
    ACTION(open, QStyle::SP_DialogOpenButton, "open project");
    ACTION(refresh, QStyle::SP_BrowserReload, "refresh");
    ACTION(terminal, "sql", "terminal");
    ACTION(save, QStyle::SP_DialogSaveButton, "save project");
    ACTION(undistort, "undistort", "undistort");
    separator1_act_ = new QAction("", this);
    separator1_act_->setSeparator(true);
    separator2_act_ = new QAction("", this);
    separator2_act_->setSeparator(true);
}

void tree_view::menu_slot(QPoint point)
{
    menu_idx_ = indexAt(point);
    QList<QAction*> acts;
    if (!match_checked([](auto& lr) { return lr.queryable; }).empty())
        acts.append(copy_checked_act_);

    auto lr = model_.get_layer(menu_idx_);
    if (lr && lr->provider) {
        if (lr->queryable) {
            acts.append(copy_act_);
            if (clipboard_.size() == 1)
                acts.append(paste_act_);
            acts.append(metadata_act_);
        }
        acts.append(attributes_act_);
        acts.append(fit_act_);
        acts.append(undistort_act_);
        acts.append(brush_color_act_);
        acts.append(pen_color_act_);
        acts.append(pen_width_act_);
        if (lr->queryable)
            acts.append(del_act_);
    }

    auto lnk = model_.get_link(menu_idx_);
    if (lnk) {
        if (lnk->queryable) {
            if (!clipboard_.empty()) {
                acts.append(paste_act_);
            }
            acts.append(terminal_act_);
        }
        acts.append(refresh_act_);
    }

    acts.append(separator1_act_);
    acts.append(attach_files_act_);
    acts.append(attach_uri_act_);
    if (lnk)
        acts.append(detach_act_);

    acts.append(separator2_act_);
    acts.append(create_act_);
    acts.append(open_act_);
    acts.append(save_act_);

    QMenu::exec(acts, mapToGlobal(point));
}

void tree_view::create_slot()
{
    model_.reset();
}

void tree_view::refresh_slot()
{
    refreshable(menu_idx_)();
}

void tree_view::open_slot()
{
    auto file_name = QFileDialog::getOpenFileName(
        this, "open project", "", "project (*.nanogis)");
    if (file_name.isEmpty())
        return;
    QFile file(file_name);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream is(&file);
        is.setVersion(QDataStream::Qt_5_6);
        is >> model_;
    }
}

void tree_view::save_slot()
{
    auto file_name = QFileDialog::getSaveFileName(
        this, "save project", "", "project (*.nanogis)");
    if (file_name.isEmpty())
        return;
    QFile file(file_name);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream os(&file);
        os.setVersion(QDataStream::Qt_5_6);
        os << model_;
    }
}

void tree_view::attach_files_slot()
{
    static const QMap<QString, QString> FilterToSchemaMap{
        {"File DSN - ODBC (*.dsn)", "odbc://?FILEDSN="},
        {"GDAL (*)", "gdal:///"},
        {"SQLite/SpatiaLite (*)", "sqlite:///"}};
    QFileDialog dlg(this, "attach file(s)");
    dlg.setNameFilters(FilterToSchemaMap.keys());
    dlg.setFileMode(QFileDialog::ExistingFiles);
    if (dlg.exec() == QDialog::Accepted) {
        auto schema = FilterToSchemaMap.value(dlg.selectedNameFilter());
        for (auto& file : dlg.selectedFiles())
            model_.link_by_uri(schema + file);
    }
}

void tree_view::attach_uri_slot()
{
    bool ok = false;
    auto uri = QInputDialog::getItem(
        this,
        "attach URI",
        "URI:",
        {"mysql://root:E207cGYM@192.168.170.128/mysql",
         "odbc://sa:E207cGYM@192.168.170.128/master?DRIVER=SQL Server;",
         "postgresql://postgres:E207cGYM@192.168.170.128/postgres"},
        0,
        true,
        &ok);
    if (ok && !uri.isEmpty())
        model_.link_by_uri(uri);
}

void tree_view::paste_slot()
{
    auto lnk = model_.get_link(menu_idx_);
    auto lr = model_.get_layer(menu_idx_);
    if (lnk) {
        auto reply = QMessageBox::question(
            this,
            "",
            "do you wish to paste?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Help);
        if (reply == QMessageBox::Yes) {
            auto tsk = std::make_shared<insertion_task>(
                clipboard_, *lnk, insertion_task::action::Execute);
            connect(tsk.get(),
                    &insertion_task::refresh_sig,
                    this,
                    refreshable(menu_idx_));
            emit task_sig(tsk);
        }
        else if (reply == QMessageBox::Help)
            emit task_sig(std::make_shared<insertion_task>(
                clipboard_, *lnk, insertion_task::action::PrintSqlOnly));
    }
    else if (lr && clipboard_.size() == 1) {
        auto tsk = std::make_shared<columns_task>(clipboard_.front(), *lr);
        connect(tsk.get(),
                &columns_task::columns_sig,
                this,
                &tree_view::insert_slot);
        runnable::start(tsk);
    }
}

void tree_view::insert_slot(layer_columns from, layer_columns to)
{
    column_matching_dialog dlg(this, from, to);
    if (dlg.exec() == QDialog::Accepted)
        emit task_sig(
            std::make_shared<insertion_task>(from.lr, to.lr, dlg.matching()));
}

void tree_view::del_slot()
{
    auto lr = model_.get_layer(menu_idx_);
    if (lr && QMessageBox::Yes ==
                  QMessageBox::question(this, "", "do you wish to delete?")) {
        auto tsk = std::make_shared<deletion_task>(*lr);
        connect(tsk.get(),
                &deletion_task::refresh_sig,
                this,
                refreshable(menu_idx_.parent()));
        emit task_sig(tsk);
    }
}

void tree_view::update_slot()
{
    menu_idx_ = {};
    emit show_sig(match_checked([](auto&) { return true; }));
};

void tree_view::copy_slot()
{
    clipboard_.clear();
    auto lr = model_.get_layer(menu_idx_);
    if (lr)
        clipboard_ = {*lr};
}

void tree_view::copy_checked_slot()
{
    clipboard_ = match_checked([](auto& lr) { return lr.queryable; });
}

void tree_view::fit_slot()
{
    auto lr = model_.get_layer(menu_idx_);
    if (lr)
        emit fit_sig(*lr);
}

void tree_view::attributes_slot()
{
    auto lr = model_.get_layer(menu_idx_);
    if (lr)
        emit attributes_sig(*lr);
}

void tree_view::metadata_slot()
{
    auto lr = model_.get_layer(menu_idx_);
    if (lr) {
        auto tsk = std::make_shared<metadata_task>(*lr);
        emit task_sig(tsk);
    }
}

void tree_view::undistort_slot()
{
    auto lr = model_.get_layer(menu_idx_);
    if (lr)
        emit undistort_sig(*lr);
}

void tree_view::brush_color_slot()
{
    auto lr = model_.get_layer(menu_idx_);
    if (!lr)
        return;
    auto color = QColorDialog::getColor(
        lr->brush.color(), this, QString(), QColorDialog::ShowAlphaChannel);
    if (color.isValid()) {
        lr->brush.setStyle(color.alpha() ? Qt::SolidPattern : Qt::NoBrush);
        lr->brush.setColor(color);
        model_.set_layer(menu_idx_, *lr);
    }
}

void tree_view::pen_color_slot()
{
    auto lr = model_.get_layer(menu_idx_);
    if (!lr)
        return;
    auto color = QColorDialog::getColor(lr->pen.color(), this);
    if (color.isValid()) {
        lr->pen.setColor(color);
        model_.set_layer(menu_idx_, *lr);
    }
}

void tree_view::pen_width_slot()
{
    auto lr = model_.get_layer(menu_idx_);
    if (!lr)
        return;
    bool ok = false;
    int width = QInputDialog::getInt(
        this, "outline width", "pixels:", lr->pen.width(), 1, 10, 1, &ok);
    if (ok) {
        lr->pen.setWidth(width);
        model_.set_layer(menu_idx_, *lr);
    }
}

void tree_view::terminal_slot()
{
    auto lnk = model_.get_link(menu_idx_);
    if (lnk)
        emit terminal_sig(*lnk);
}

void tree_view::detach_slot()
{
    auto lnk = model_.get_link(menu_idx_);
    if (lnk)
        model_.removeRows(menu_idx_.row(), 1);
}
