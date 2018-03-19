// Andrew Naplavkov

#include "task_widget.h"
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QToolBar>
#include <QVBoxLayout>
#include <chrono>

task_widget::task_widget(QWidget* parent, task_ptr tsk)
    : QWidget(parent), tsk_(tsk)
{
    auto tools = new QToolBar;
    tools->setFloatable(false);
    tools->setMovable(false);
    tools->addAction(style()->standardIcon(QStyle::SP_DialogCloseButton),
                     "close",
                     this,
                     &task_widget::close_slot);
    edit_ = new QPlainTextEdit;
    QPalette palette;
    palette.setColor(QPalette::Base, Qt::black);
    palette.setColor(QPalette::Text, Qt::green);
    edit_->setPalette(palette);
    edit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    edit_->setLineWrapMode(QPlainTextEdit::NoWrap);
    edit_->setWordWrapMode(QTextOption::NoWrap);
    edit_->setReadOnly(true);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(tools);
    layout->addWidget(edit_);
    setLayout(layout);
}

void task_widget::start()
{
    using namespace std::chrono;
    runnable::start(tsk_);
    timer_.start(duration_cast<milliseconds>(bark::UiTimeout).count(), this);
}

void task_widget::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == timer_.timerId()) {
        auto out = tsk_->pop_output();
        if (!out.isEmpty())
            edit_->appendPlainText(out);
        auto status = tsk_->state();
        if (status == task::status::Failed ||
            status == task::status::Successed) {
            emit idle_sig(this);
            timer_.stop();
        }
    }
    else {
        QWidget::timerEvent(event);
    }
}

void task_widget::close_slot()
{
    tsk_->cancel();
    emit close_sig(this);
}
