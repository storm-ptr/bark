// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_POOL_HPP
#define BARK_DB_DETAIL_POOL_HPP

#include <algorithm>
#include <bark/db/command.hpp>
#include <exception>
#include <mutex>
#include <queue>
#include <thread>

namespace bark::db::detail {

/// thread-safe reuse interface to prevent the connection time overhead
class pool : public std::enable_shared_from_this<pool> {
public:
    explicit pool(command_allocator alloc) : alloc_{std::move(alloc)} {}

    command_holder make_command()
    {
        auto self = shared_from_this();
        auto deleter = [self](command* cmd) { self->push(cmd); };
        auto holder = command_holder(nullptr, deleter);
        std::lock_guard lock{guard_};
        if (commands_.empty())
            holder.reset(alloc_());
        else {
            holder.reset(commands_.front().release());
            commands_.pop();  // no-throw guarantee
        }
        return holder;
    }

private:
    std::mutex guard_;
    command_allocator alloc_;
    std::queue<command_holder> commands_;

    void push(command* cmd) try {
        static const auto ConnectionsLimit =
            std::max<size_t>(2, std::thread::hardware_concurrency());
        if (!cmd)
            return;
        auto holder = command_holder(cmd, std::default_delete<command>());
        cmd->set_autocommit(true);
        std::lock_guard lock{guard_};
        if (commands_.size() < ConnectionsLimit)
            commands_.emplace(std::move(holder));
    }
    catch (const std::exception&) {
    }
};

}  // namespace bark::db::detail

#endif  // BARK_DB_DETAIL_POOL_HPP
