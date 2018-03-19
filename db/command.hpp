// Andrew Naplavkov

#ifndef BARK_DB_COMMAND_HPP
#define BARK_DB_COMMAND_HPP

#include <bark/dataset/ostream.hpp>
#include <bark/dataset/rowset.hpp>
#include <bark/db/sql_builder.hpp>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bark {
namespace db {

/// NOT thread-safe low-level interface
struct command {
    virtual ~command() = default;
    virtual sql_syntax syntax() = 0;
    virtual command& exec(const sql_builder&) = 0;
    virtual std::vector<std::string> columns() = 0;
    virtual bool fetch(dataset::ostream&) = 0;
    virtual void set_autocommit(bool) = 0;
    virtual void commit() = 0;
};

inline auto builder(command& cmd)
{
    return sql_builder{cmd.syntax()};
}

inline command& exec(command& cmd, const sql_builder& bld)
{
    return cmd.exec(bld);
}

template <typename T>
command& exec(command& cmd, const T& sql)
{
    return cmd.exec(builder(cmd) << sql);
}

inline dataset::rowset fetch_all(command& cmd)
{
    auto cols = cmd.columns();
    dataset::ostream os;
    while (cmd.fetch(os))
        ;
    return {std::move(cols), std::move(os).buf()};
}

template <typename T>
T fetch_or_default(command& cmd)
{
    auto rows = fetch_all(cmd);
    return rows.empty() ? T{} : boost::lexical_cast<T>((*rows.begin())[0]);
}

using command_allocator = std::function<command*()>;
using command_deleter = std::function<void(command*)>;
using command_holder = std::unique_ptr<command, command_deleter>;

}  // namespace db
}  // namespace bark

#endif  // BARK_DB_COMMAND_HPP
