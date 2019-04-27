// Andrew Naplavkov

#ifndef COLUMN_MATCHING_DIALOG_H
#define COLUMN_MATCHING_DIALOG_H

#include "tools.h"
#include <QDialog>
#include <QGridLayout>
#include <QObject>
#include <QWidget>

class column_matching_dialog : public QDialog {
    Q_OBJECT

public:
    column_matching_dialog(QWidget* parent,
                           const layer_columns& from,
                           const layer_columns& to);

    string_map matching() const;

private:
    QGridLayout* grid_;
};

#endif  // COLUMN_MATCHING_DIALOG_H
