// Andrew Naplavkov

#ifndef BARK_DB_POOL_HPP
#define BARK_DB_POOL_HPP

#include <algorithm>
#include <bark/db/command.hpp>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace bark::db {

/// thread-safe reuse interface to prevent the connection time overhead
class pool : public std::enable_shared_from_this<pool> {
public:
    explicit pool(std::function<command*()> alloc) : alloc_{std::move(alloc)} {}

    command_holder make_command()
    {
        auto self = shared_from_this();
        auto deleter = [self](command* cmd) { self->push(cmd); };
        auto res = command_holder(nullptr, deleter);
        std::lock_guard lock{guard_};
        if (commands_.empty())
            res.reset(alloc_());
        else {
            res.reset(commands_.front().release());
            commands_.pop();  // no-throw guarantee
        }
        return res;
    }

private:
    std::mutex guard_;
    std::function<command*()> alloc_;
    std::queue<command_holder> commands_;

    void push(command* cmd) try {
        static const auto Limit =
            std::max<size_t>(2, std::thread::hardware_concurrency());
        if (!cmd)
            return;
        auto holder = command_holder(cmd, std::default_delete<command>());
        cmd->set_autocommit(true);
        std::lock_guard lock{guard_};
        if (commands_.size() < Limit)
            commands_.emplace(std::move(holder));
    }
    catch (const std::exception&) {
    }
};

}  // namespace bark::db

#endif  // BARK_DB_POOL_HPP
