// Andrew Naplavkov

#ifndef OUTPUT_WIDGET_H
#define OUTPUT_WIDGET_H

#include "task.h"
#include <QBasicTimer>
#include <QPlainTextEdit>
#include <QWidget>

class task_widget : public QWidget {
    Q_OBJECT

public:
    task_widget(QWidget*, task_ptr);
    void start();

signals:
    void close_sig(QWidget*);
    void idle_sig(QWidget*);

protected:
    void timerEvent(QTimerEvent*) override;

private slots:
    void close_slot();

private:
    task_ptr tsk_;
    QPlainTextEdit* edit_;
    QBasicTimer timer_;
};

#endif  // OUTPUT_WIDGET_H
