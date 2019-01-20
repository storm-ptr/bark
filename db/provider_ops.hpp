// Andrew Naplavkov

#ifndef BARK_DB_PROVIDER_OPS_HPP
#define BARK_DB_PROVIDER_OPS_HPP

#include <algorithm>
#include <bark/db/detail/utility.hpp>
#include <bark/db/provider.hpp>
#include <bark/db/sql_builder_ops.hpp>
#include <bark/db/table_def_ops.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <exception>

namespace bark::db {

inline layer_type type(provider& pvd, const qualified_name& layer)
{
    return pvd.dir()[layer];
}

inline column_def column(provider& pvd, const qualified_name& col_nm)
{
    return column(pvd.table(qualifier(col_nm)).columns, col_nm.back());
}

inline bool queryable(provider& pvd) try {
    pvd.make_command();
    return true;
}
catch (const std::exception&) {
    return false;
}

inline sql_builder builder(provider& pvd)
{
    return builder(*pvd.make_command());
}

template <class T>
void exec(provider& pvd, const T& sql)
{
    exec(*pvd.make_command(), sql);
}

template <class Sql>
auto fetch_all(provider& pvd, const Sql& sql)
{
    return fetch_all(exec(*pvd.make_command(), sql));
}

template <class T, class Sql>
T fetch_or_default(provider& pvd, const Sql& sql)
{
    return fetch_or_default<T>(exec(*pvd.make_command(), sql));
}

template <class ColumnNames>
sql_builder select_sql(provider& pvd,
                       const qualified_name& tbl_nm,
                       const ColumnNames& col_nms)
{
    auto res = builder(pvd);
    auto tbl = pvd.table(tbl_nm);
    auto cols = columns(tbl.columns, col_nms);
    res << "SELECT " << list{cols, ", ", [](auto& col) { return decoder{col}; }}
        << " FROM " << tbl_nm;
    return res;
}

template <class ColumnNames>
sql_builder select_sql(provider& pvd,
                       const qualified_name& tbl_nm,
                       const ColumnNames& col_nms,
                       size_t offset,
                       size_t limit)
{
    using namespace boost::adaptors;
    auto res = select_sql(pvd, tbl_nm, col_nms);
    auto tbl = pvd.table(tbl_nm);
    auto pri = find_primary(tbl.indexes);
    res << " ORDER BY ";
    if (pri != tbl.indexes.end())
        res << list{pri->columns, ", ", id<>};
    else
        res << list{
            tbl.columns | filtered(sortable) | transformed(name), ", ", id<>};
    res << " ";
    pvd.page_clause(res, offset, limit);
    return res;
}

template <class ColumnNames, class... Args>
rowset select(provider& pvd,
              const qualified_name& tbl_nm,
              const ColumnNames& col_nms,
              Args&&... args)
{
    auto bld = select_sql(pvd, tbl_nm, col_nms, std::forward<Args>(args)...);
    auto res = fetch_all(pvd, bld);
    if (!std::equal(std::begin(col_nms),
                    std::end(col_nms),
                    res.columns.begin(),
                    unicode::case_insensitive_equal_to{}))
        res = select({std::begin(col_nms), std::end(col_nms)}, res);
    return res;
}

template <class ColumnNames, class Rows>
sql_builder insert_sql(provider& pvd,
                       const qualified_name& tbl_nm,
                       const ColumnNames& col_nms,
                       const Rows& rows)
{
    auto res = builder(pvd);
    auto tbl = pvd.table(tbl_nm);
    auto cols = columns(tbl.columns, col_nms);
    auto encode = [&](auto& row) { return row_encoder{cols, row}; };
    res << "INSERT INTO " << tbl_nm << " (" << list{col_nms, ", ", id<>}
        << ") VALUES\n"
        << list{rows, ",\n", encode};
    return res;
};

inline sql_builder drop_sql(provider& pvd, const qualified_name& tbl_nm)
{
    auto res = builder(pvd);
    res << "DROP TABLE " << tbl_nm;
    return res;
}

}  // namespace bark::db

#endif  // BARK_DB_PROVIDER_OPS_HPP
