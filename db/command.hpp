// Andrew Naplavkov

#ifndef BARK_DB_COMMAND_HPP
#define BARK_DB_COMMAND_HPP

#include <bark/db/fwd.hpp>
#include <bark/db/rowset.hpp>
#include <bark/db/sql_builder.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>

namespace bark::db {

/// Not thread-safe interface to execute SQL statements
struct command {
    virtual ~command() = default;

    /// Option for constructing @ref sql_builder
    virtual sql_quoted_identifier quoted_identifier() = 0;

    /// Option for constructing @ref sql_builder
    virtual sql_parameter_marker parameter_marker() = 0;

    /// Executes the SQL in @ref sql_builder
    virtual void exec(const sql_builder&) = 0;

    /// Returns the field information for the current query
    virtual std::vector<std::string> columns() = 0;

    /// @ref variant_ostream is populated with the row's values
    virtual bool fetch(variant_ostream&) = 0;

    /// By default, each statement is automatically committed
    virtual void set_autocommit(bool) = 0;

    /// Commits a transaction to the database
    virtual void commit() = 0;
};

inline sql_builder builder(command& cmd)
{
    return sql_builder{cmd.quoted_identifier(), cmd.parameter_marker()};
}

inline void exec(command& cmd, const sql_builder& bld)
{
    cmd.exec(bld);
}

template <class Sql>
void exec(command& cmd, const Sql& sql)
{
    cmd.exec(builder(cmd) << sql);
}

inline rowset fetch_all(command& cmd)
{
    auto cols = cmd.columns();
    variant_ostream os;
    while (cmd.fetch(os))
        ;
    return {std::move(cols), std::move(os.data)};
}

template <class Result>
auto fetch_or(command& cmd, const Result& val)
{
    auto rows = fetch_all(cmd);
    auto is = variant_istream{rows.data};
    return is.data.empty() ? val : boost::lexical_cast<Result>(read(is));
}

}  // namespace bark::db

#endif  // BARK_DB_COMMAND_HPP
