// Andrew Naplavkov

#ifndef SQL_WIDGET_H
#define SQL_WIDGET_H

#include "task.h"
#include <QPlainTextEdit>
#include <QWidget>

class sql_widget : public QWidget {
    Q_OBJECT

public:
    sql_widget(QWidget*, bark::qt::link);

signals:
    void close_sig(QWidget*);
    void run_sig(task_ptr);

public slots:

private slots:
    void close_slot();
    void run_slot();

private:
    bark::qt::link lnk_;
    QPlainTextEdit* edit_;
};

#endif  // SQL_WIDGET_H
