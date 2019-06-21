// Andrew Naplavkov

#ifndef BARK_DB_FWD_HPP
#define BARK_DB_FWD_HPP

#include <functional>
#include <memory>

namespace bark::db {

struct command;

using command_holder = std::unique_ptr<command, std::function<void(command*)>>;

struct provider;

using provider_ptr = std::shared_ptr<provider>;

}  // namespace bark::db

#endif  // BARK_DB_FWD_HPP
