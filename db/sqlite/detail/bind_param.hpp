// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_DETAIL_BIND_PARAM_HPP
#define BARK_DB_SQLITE_DETAIL_BIND_PARAM_HPP

#include <bark/db/rowset.hpp>
#include <bark/db/sqlite/detail/utility.hpp>

namespace bark::db::sqlite::detail {

inline int bind_param(const variant_t& v, sqlite3_stmt* stmt, int order)
{
    ++order;  // one-based
    return std::visit(
        overloaded{
            [&](std::monostate) { return sqlite3_bind_null(stmt, order); },
            [&](int64_t v) { return sqlite3_bind_int64(stmt, order, v); },
            [&](double v) { return sqlite3_bind_double(stmt, order, v); },
            [&](std::string_view v) {
                return sqlite3_bind_text(
                    stmt, order, v.data(), int(v.size()), SQLITE_TRANSIENT);
            },
            [&](blob_view v) {
                return sqlite3_bind_blob(
                    stmt, order, v.data(), int(v.size()), SQLITE_TRANSIENT);
            }},
        v);
}

}  // namespace bark::db::sqlite::detail

#endif  // BARK_DB_SQLITE_DETAIL_BIND_PARAM_HPP
