// Andrew Naplavkov

#ifndef BARK_DB_COMMAND_OPS_HPP
#define BARK_DB_COMMAND_OPS_HPP

#include <bark/db/command.hpp>
#include <bark/detail/unicode.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/find_if.hpp>

namespace bark::db {

/// Specifies the columns to be returned.
inline rowset select(std::vector<std::string> columns, const rowset& from)
{
    auto positions = as<std::vector<size_t>>(columns, [&](auto& lhs) {
        auto it = boost::range::find_if(from.columns, [&](auto& rhs) {
            return unicode::case_insensitive_equal_to{}(lhs, rhs);
        });
        return std::distance(from.columns.begin(), it);
    });
    variant_ostream os;
    for (auto& row : range(from))
        for (auto pos : positions)
            os << row.at(pos);
    return {std::move(columns), std::move(os.data)};
}

inline sql_syntax embeded_params(sql_syntax syntax)
{
    syntax.parameter_marker = nullptr;
    return syntax;
}

template <class T>
T fetch_or_default(command& cmd)
{
    auto rows = fetch_all(cmd);
    auto is = variant_istream{rows.data};
    return is.data.empty() ? T{} : boost::lexical_cast<T>(read(is));
}

}  // namespace bark::db

#endif  // BARK_DB_COMMAND_OPS_HPP
