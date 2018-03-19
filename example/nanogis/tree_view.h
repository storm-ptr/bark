// Andrew Naplavkov

#ifndef TREE_VIEW_H
#define TREE_VIEW_H

#include "task.h"
#include <QAction>
#include <QIcon>
#include <QModelIndex>
#include <QPoint>
#include <QString>
#include <QTreeView>
#include <QVector>
#include <QWidget>
#include <bark/qt/tree_model.hpp>

class tree_view : public QTreeView {
    Q_OBJECT

public:
    explicit tree_view(QWidget* parent);

signals:
    void attributes_sig(bark::qt::layer);
    void fit_sig(bark::qt::layer);
    void show_sig(QVector<bark::qt::layer>);
    void task_sig(task_ptr);
    void terminal_sig(bark::qt::link);
    void undistort_sig(bark::qt::layer);

private slots:
    void attach_files_slot();
    void attach_uri_slot();
    void attributes_slot();
    void brush_color_slot();
    void copy_slot();
    void copy_checked_slot();
    void create_slot();
    void del_slot();
    void detach_slot();
    void fit_slot();
    void insert_slot(layer_columns from, layer_columns to);
    void menu_slot(QPoint);
    void metadata_slot();
    void open_slot();
    void paste_slot();
    void pen_color_slot();
    void pen_width_slot();
    void refresh_slot();
    void save_slot();
    void terminal_slot();
    void undistort_slot();
    void update_slot();

private:
    bark::qt::tree_model model_;
    QModelIndex menu_idx_;
    QVector<bark::qt::layer> clipboard_;
    QAction* attach_files_act_;
    QAction* attach_uri_act_;
    QAction* attributes_act_;
    QAction* brush_color_act_;
    QAction* copy_act_;
    QAction* copy_checked_act_;
    QAction* create_act_;
    QAction* del_act_;
    QAction* detach_act_;
    QAction* fit_act_;
    QAction* metadata_act_;
    QAction* open_act_;
    QAction* paste_act_;
    QAction* pen_color_act_;
    QAction* pen_width_act_;
    QAction* refresh_act_;
    QAction* save_act_;
    QAction* separator1_act_;
    QAction* separator2_act_;
    QAction* terminal_act_;
    QAction* undistort_act_;

    template <typename Filter>
    QVector<bark::qt::layer> match_checked(Filter filter) const
    {
        QVector<bark::qt::layer> res;
        for (auto& idx : model_.match({},
                                      Qt::CheckStateRole,
                                      QVariant::fromValue(Qt::Checked),
                                      -1,
                                      Qt::MatchRecursive)) {
            auto lr = model_.get_layer(idx);
            if (lr && lr->provider && filter(*lr))
                res.push_back(*lr);
        }
        return res;
    }

    auto refreshable(QModelIndex idx)
    {
        return [this, displayRole = model_.data(idx, Qt::DisplayRole)] {
            for (auto& idx : model_.match({},
                                          Qt::DisplayRole,
                                          displayRole,
                                          -1,
                                          Qt::MatchRecursive)) {
                auto lnk = model_.get_link(idx);
                if (lnk)
                    model_.link_by_uri(lnk->uri);
            }
        };
    }
};

#endif  // TREE_VIEW_H
