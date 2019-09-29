// Andrew Naplavkov

#ifndef BARK_QT_START_THREAD_HPP
#define BARK_QT_START_THREAD_HPP

#include <QRunnable>
#include <QThreadPool>
#include <bark/db/provider.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace bark::qt {

constexpr int PriorityNormal{0};

struct cancel_exception : std::runtime_error {
    cancel_exception() : std::runtime_error{"canceled"} {}
};

class runnable : public QRunnable {
public:
    template <class Functor>
    runnable(Functor&& f, int priority)
        : task_{std::forward<Functor>(f)}, priority_{priority}
    {
    }

    void run() override;

private:
    std::function<void()> task_;
    const int priority_;
};

template <class Functor>
void start_thread(Functor&& f, int priority = PriorityNormal)
{
    auto ptr = new runnable(std::forward<Functor>(f), priority);
    ptr->setAutoDelete(true);
    QThreadPool::globalInstance()->start(ptr, priority);
}

inline void runnable::run()
try {
    task_();
}
catch (const db::busy_exception&) {
    start_thread(std::move(task_), priority_ - 1);
}
catch (const cancel_exception&) {
    // ignore
}
catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
}

}  // namespace bark::qt

#endif  // BARK_QT_START_THREAD_HPP
