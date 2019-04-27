// Andrew Naplavkov

#include "sql_widget.h"
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QToolBar>
#include <QVBoxLayout>

sql_widget::sql_widget(QWidget* parent, bark::qt::link lnk)
    : QWidget(parent), lnk_(std::move(lnk))
{
    auto tools = new QToolBar;
    tools->setFloatable(false);
    tools->setMovable(false);
    tools->addAction(style()->standardIcon(QStyle::SP_DialogCloseButton),
                     "close",
                     this,
                     SLOT(close_slot()));
    tools->addAction(style()->standardIcon(QStyle::SP_MediaPlay),
                     "run",
                     this,
                     SLOT(run_slot()));
    auto title = new QLabel(lnk_.uri.toDisplayString(QUrl::DecodeReserved));
    edit_ = new QPlainTextEdit;
    edit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    edit_->setTabChangesFocus(true);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(tools);
    layout->addWidget(title);
    layout->addWidget(edit_);
    setLayout(layout);
}

void sql_widget::close_slot()
{
    emit close_sig(this);
}

void sql_widget::run_slot()
{
    emit run_sig(
        std::make_shared<sql_task>(lnk_, edit_->toPlainText().toStdString()));
}
