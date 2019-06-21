// Andrew Naplavkov

#ifndef BARK_DB_COMMAND_HPP
#define BARK_DB_COMMAND_HPP

#include <bark/db/fwd.hpp>
#include <bark/db/rowset.hpp>
#include <bark/db/sql_builder.hpp>
#include <string>
#include <vector>

namespace bark::db {

/// Not thread-safe interface to execute SQL statements
struct command {
    virtual ~command() = default;

    /// Returns the options for constructing @ref sql_builder
    virtual sql_syntax syntax() = 0;

    /// Executes the SQL in @ref sql_builder
    virtual command& exec(const sql_builder&) = 0;

    /// Returns the field information for the current query
    virtual std::vector<std::string> columns() = 0;

    /// @ref variant_ostream is populated with the row's values
    virtual bool fetch(variant_ostream&) = 0;

    /// By default, each statement is automatically committed
    virtual command& set_autocommit(bool) = 0;

    /// Commits a transaction to the database
    virtual command& commit() = 0;
};

inline sql_builder builder(command& cmd)
{
    return sql_builder{cmd.syntax()};
}

inline command& exec(command& cmd, const sql_builder& bld)
{
    return cmd.exec(bld);
}

template <class T>
command& exec(command& cmd, const T& sql)
{
    return cmd.exec(builder(cmd) << sql);
}

inline rowset fetch_all(command& cmd)
{
    auto cols = cmd.columns();
    variant_ostream os;
    while (cmd.fetch(os))
        ;
    return {std::move(cols), std::move(os.data)};
}

}  // namespace bark::db

#endif  // BARK_DB_COMMAND_HPP
