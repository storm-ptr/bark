// Andrew Naplavkov

#ifndef BARK_DB_PROVIDER_HPP
#define BARK_DB_PROVIDER_HPP

#include <algorithm>
#include <bark/dataset/rowset_ops.hpp>
#include <bark/db/command.hpp>
#include <bark/db/detail/common.hpp>
#include <bark/db/fwd.hpp>
#include <bark/db/qualified_name.hpp>
#include <bark/db/sql_builder_ops.hpp>
#include <bark/db/table_def_ops.hpp>
#include <bark/detail/lru_cache.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <exception>
#include <map>
#include <memory>

namespace bark {
namespace db {

using layer_to_type_map = std::map<qualified_name, layer_type>;

using busy_exception = bark::detail::lru_cache::busy_exception;

struct table_script {
    qualified_name name;
    std::string sql;
};

/// thread-safe high-level interface
struct provider {
    virtual ~provider() = default;

    virtual layer_to_type_map dir() = 0;

    virtual std::string projection(const qualified_name& layer) = 0;

    virtual geometry::box extent(const qualified_name& layer) = 0;

    virtual double undistorted_scale(const qualified_name& layer,
                                     const geometry::view&) = 0;

    virtual geometry::multi_box tile_coverage(const qualified_name& layer,
                                              const geometry::view&) = 0;

    /// @return {<wkb[, image][, attributes]>}
    virtual dataset::rowset spatial_objects(const qualified_name& layer,
                                            const geometry::view&) = 0;

    virtual command_holder make_command() = 0;

    virtual table_def table(const qualified_name& tbl_nm) = 0;

    virtual table_script script(const table_def&) = 0;

    virtual void page_clause(sql_builder&, size_t offset, size_t limit) = 0;

    virtual void refresh() = 0;
};

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

template <typename T>
void exec(provider& pvd, const T& sql)
{
    exec(*pvd.make_command(), sql);
}

template <typename Sql>
dataset::rowset fetch_all(provider& pvd, const Sql& sql)
{
    return fetch_all(exec(*pvd.make_command(), sql));
}

template <typename T, typename Sql>
T fetch_or_default(provider& pvd, const Sql& sql)
{
    return fetch_or_default<T>(exec(*pvd.make_command(), sql));
}

inline sql_builder drop_sql(provider& pvd, const qualified_name& tbl_nm)
{
    auto res = builder(pvd);
    res << "DROP TABLE " << tbl_nm;
    return res;
}

inline void drop(provider& pvd, const qualified_name& tbl_nm)
{
    exec(pvd, drop_sql(pvd, tbl_nm));
}

template <typename ColumnNames>
sql_builder select_sql(provider& pvd,
                       const qualified_name& tbl_nm,
                       const ColumnNames& col_nms)
{
    auto res = builder(pvd);
    auto tbl = pvd.table(tbl_nm);
    auto cols = columns(tbl.columns, col_nms);
    res << "SELECT " << list(cols, ", ", decode) << " FROM " << tbl_nm;
    return res;
}

template <typename ColumnNames>
sql_builder select_sql(provider& pvd,
                       const qualified_name& tbl_nm,
                       const ColumnNames& col_nms,
                       size_t offset,
                       size_t limit)
{
    auto res = select_sql(pvd, tbl_nm, col_nms);
    auto tbl = pvd.table(tbl_nm);
    auto pri = find_primary(tbl.indexes);
    res << " ORDER BY ";
    if (pri != tbl.indexes.end())
        res << list(pri->columns, ", ", id<>);
    else
        res << list(
            column_names(tbl.columns | boost::adaptors::filtered([](auto& col) {
                             switch (col.type) {
                                 case column_type::Integer:
                                 case column_type::Real:
                                 case column_type::Text:
                                     return true;
                                 default:
                                     return false;
                             }
                         })),
            ", ",
            id<>);
    res << " ";
    pvd.page_clause(res, offset, limit);
    return res;
}

template <typename ColumnNames, typename... Args>
dataset::rowset select(provider& pvd,
                       const qualified_name& tbl_nm,
                       const ColumnNames& col_nms,
                       Args&&... args)
{
    auto bld = select_sql(pvd, tbl_nm, col_nms, std::forward<Args>(args)...);
    auto res = fetch_all(pvd, bld);
    if (!std::equal(std::begin(col_nms),
                    std::end(col_nms),
                    res.columns().begin(),
                    unicode::case_insensitive_equal_to{}))
        res = dataset::select(col_nms, res);
    return res;
}

template <typename ColumnNames, typename Rows>
sql_builder insert_sql(provider& pvd,
                       const qualified_name& tbl_nm,
                       const ColumnNames& col_nms,
                       const Rows& rows)
{
    auto res = builder(pvd);
    auto tbl = pvd.table(tbl_nm);
    auto cols = columns(tbl.columns, col_nms);
    res << "INSERT INTO " << tbl_nm << " (" << list(col_nms, ", ", id<>)
        << ") VALUES\n"
        << list(rows, ",\n", row_value_constructor(cols));
    return res;
};

}  // namespace db
}  // namespace bark

#endif  // BARK_DB_PROVIDER_HPP
