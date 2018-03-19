// Andrew Naplavkov

#ifndef TASK_H
#define TASK_H

#include "tools.h"
#include <QObject>
#include <QRunnable>
#include <QString>
#include <QTextStream>
#include <memory>
#include <mutex>

class task : public QObject {
public:
    enum class status { Waiting, Running, Canceling, Failed, Successed };

    task();
    status state();
    void push_output(const QString&);
    QString pop_output();
    void cancel();
    void run();

protected:
    virtual void run_event() = 0;

private:
    std::mutex guard_;
    status state_;
    QString str_;
    QTextStream output_;
};

void push_output(task& tsk, const std::string& str);

using task_ptr = std::shared_ptr<task>;

class runnable : public QRunnable {
public:
    void run() override;
    static void start(task_ptr);

private:
    task_ptr tsk_;

    explicit runnable(task_ptr);
};

class columns_task : public task {
    Q_OBJECT

public:
    columns_task(bark::qt::layer, bark::qt::layer);

signals:
    void columns_sig(layer_columns, layer_columns);

protected:
    void run_event() override;

private:
    bark::qt::layer lhs_;
    bark::qt::layer rhs_;
};

class deletion_task : public task {
    Q_OBJECT

public:
    explicit deletion_task(bark::qt::layer);

signals:
    void refresh_sig();

protected:
    void run_event() override;

private:
    bark::qt::layer lr_;
};

class sql_task : public task {
public:
    sql_task(bark::qt::link, std::string);

protected:
    void run_event() override;

private:
    bark::qt::link lnk_;
    std::string sql_;
};

class attributes_task : public task {
public:
    attributes_task(bark::qt::layer, bark::qt::frame);

protected:
    void run_event() override;

private:
    bark::qt::layer lr_;
    bark::qt::frame frm_;
};

class metadata_task : public task {
public:
    explicit metadata_task(bark::qt::layer);

protected:
    void run_event() override;

private:
    bark::qt::layer lr_;
};

#endif  // TASK_H
