// Andrew Naplavkov

#include "column_matching_dialog.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

column_matching_dialog::column_matching_dialog(QWidget* parent,
                                               layer_columns from,
                                               layer_columns to)
    : QDialog(parent)
{
    setWindowTitle("match columns to paste");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    grid_ = new QGridLayout;
    for (int i = 0; i < (int)to.cols.size(); ++i) {
        auto col_to = QString::fromStdString(to.cols[i]);
        auto lbl = new QLabel;
        lbl->setText(col_to);
        grid_->addWidget(lbl, i, 0, Qt::AlignRight);

        auto combo = new QComboBox;
        combo->addItem(QString{});
        for (auto& col_from : from.cols)
            combo->addItem(QString::fromStdString(col_from));
        int pos = combo->findText(col_to, Qt::MatchFixedString);
        if (pos >= 0)
            combo->setCurrentIndex(pos);
        grid_->addWidget(combo, i, 1);
    }

    auto buttons = new QHBoxLayout;

    auto yes_btn = new QPushButton("Yes");
    connect(yes_btn, &QPushButton::clicked, this, &QDialog::accept);
    buttons->addWidget(yes_btn);

    auto no_btn = new QPushButton("No");
    connect(no_btn, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(no_btn);

    auto layout = new QVBoxLayout;
    layout->addLayout(grid_);
    layout->addLayout(buttons);
    setLayout(layout);
}

string_map column_matching_dialog::matching() const
{
    string_map res;
    for (int row(0); row < grid_->rowCount(); ++row) {
        auto from =
            static_cast<QLabel*>(grid_->itemAtPosition(row, 0)->widget())
                ->text()
                .toStdString();
        auto to =
            static_cast<QComboBox*>(grid_->itemAtPosition(row, 1)->widget())
                ->currentText()
                .toStdString();
        if (to.empty())
            continue;
        res.insert({from, to});
    }
    return res;
}
