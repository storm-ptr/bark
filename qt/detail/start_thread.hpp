// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_START_THREAD_HPP
#define BARK_QT_DETAIL_START_THREAD_HPP

#include <QRunnable>
#include <QThreadPool>
#include <bark/db/provider.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace bark {
namespace qt {
namespace detail {

constexpr int PriorityNormal{0};

struct cancel_exception : std::runtime_error {
    cancel_exception() : std::runtime_error{"canceled"} {}
};

class runnable : public QRunnable {
public:
    template <typename Functor>
    runnable(Functor&& fn, int priority)
        : task_{std::forward<Functor>(fn)}, priority_{priority}
    {
    }

    void run() override;

private:
    std::function<void()> task_;
    const int priority_;
};

template <typename Functor>
void start_thread(Functor&& fn, int priority = PriorityNormal)
{
    auto ptr = new runnable(std::forward<Functor>(fn), priority);
    ptr->setAutoDelete(true);
    QThreadPool::globalInstance()->start(ptr, priority);
}

inline void runnable::run() try {
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

}  // namespace detail
}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_START_THREAD_HPP
