// Andrew Naplavkov

#ifndef INSERTION_TASK_H
#define INSERTION_TASK_H

#include "task.h"
#include "tools.h"
#include <QVector>
#include <functional>

class insertion_task : public task {
    Q_OBJECT

public:
    enum class action { Execute, PrintSqlOnly };

    insertion_task(QVector<bark::qt::layer> from,
                   bark::qt::link to,
                   action act);
    insertion_task(bark::qt::layer from, bark::qt::layer to, string_map cols);

signals:
    void refresh_sig();

protected:
    void run_event() override;

private:
    using function_type = std::function<void(insertion_task*)>;

    function_type f_;

    bark::qt::layer create(bark::qt::layer from, bark::qt::link to);
    void insert(bark::qt::layer from, bark::qt::layer to, string_map cols);
};

#endif  // INSERTION_TASK_H
