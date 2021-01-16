// Andrew Naplavkov

#ifndef BARK_DB_PROVIDER_HPP
#define BARK_DB_PROVIDER_HPP

#include <bark/db/command.hpp>
#include <bark/db/detail/meta_ops.hpp>
#include <bark/db/fwd.hpp>
#include <bark/detail/lru_cache.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/combine.hpp>
#include <map>

namespace bark::db {

using busy_exception = lru_cache::busy_exception;

/// Thread-safe, caching interface for spatial data source
struct provider {
    virtual ~provider() = default;

    /// Returns layers in this data source
    virtual std::map<qualified_name, meta::layer_type> dir() = 0;

    /// Returns PROJ.4 string for the spatial reference system
    virtual std::string projection(const qualified_name& layer) = 0;

    /// Returns the minimum bounding rectangle of the data set
    virtual geometry::box extent(const qualified_name& layer) = 0;

    /// Changes the pixel to undistort the raster
    virtual geometry::box undistorted_pixel(const qualified_name& layer,
                                            const geometry::box& pixel) = 0;

    /// Returns a balanced data grid.

    /// @param layer is a data set identifier;
    /// @param extent is a spatial filter;
    /// @param pixel selects the level of the raster pyramid.
    virtual geometry::multi_box tile_coverage(const qualified_name& layer,
                                              const geometry::box& extent,
                                              const geometry::box& pixel) = 0;

    /// Returns @ref rowset with spatial data set.

    /// Columns @code GEOMETRY[,IMAGE][,ATTRIBUTES...] @endcode
    /// @param layer is a data set identifier;
    /// @param extent is a spatial filter;
    /// @param pixel selects the level of the raster pyramid.
    virtual rowset spatial_objects(const qualified_name& layer,
                                   const geometry::box& extent,
                                   const geometry::box& pixel) = 0;

    /// Returns SQL command interface
    virtual command_holder make_command() = 0;

    /// Describes table
    virtual meta::table table(const qualified_name& tbl_nm) = 0;

    /// Returns name and DDL command to create a table
    virtual std::pair<qualified_name, std::string> ddl(const meta::table&) = 0;

    /// Outputs SQL LIMIT clause
    virtual void page_clause(sql_builder&, size_t offset, size_t limit) = 0;

    /// Resets LRU cache
    virtual void refresh() = 0;
};

inline bool queryable(provider& pvd)
try {
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

template <class Sql>
void exec(provider& pvd, Sql&& sql)
{
    exec(*pvd.make_command(), std::forward<Sql>(sql));
}

template <class Sql>
rowset fetch_all(provider& pvd, Sql&& sql)
{
    auto cmd = pvd.make_command();
    exec(*cmd, std::forward<Sql>(sql));
    return fetch_all(*cmd);
}

inline sql_builder select_sql(provider& pvd,
                              const qualified_name& tbl_nm,
                              size_t offset,
                              size_t limit)
{
    auto res = builder(pvd);
    auto tbl = pvd.table(tbl_nm);
    auto pri =
        boost::range::find_if(tbl.indexes, same{meta::index_type::Primary});
    res << "SELECT " << list{tbl.columns, ", ", decode} << " FROM " << tbl_nm
        << " ORDER BY "
        << list{pri != tbl.indexes.end()
                    ? pri->columns
                    : names(tbl.columns | boost::adaptors::filtered(sortable)),
                ", ",
                id<>}
        << " ";
    pvd.page_clause(res, offset, limit);
    return res;
}

template <class ColumnNames, class Rows>
sql_builder insert_sql(provider& pvd,
                       const qualified_name& tbl_nm,
                       const ColumnNames& col_nms,
                       const Rows& rows)
{
    auto tbl = pvd.table(tbl_nm);
    auto cols = as<std::vector<meta::column>>(col_nms, [&](const auto& col_nm) {
        return *db::find(tbl.columns, col_nm);
    });
    auto encode = [&](const auto& row) {
        return streamable([&](sql_builder& bld) {
            bld << "("
                << list{boost::combine(cols, row),
                        ",",
                        [](const auto& pair) {
                            return db::encode(boost::get<0>(pair),
                                              boost::get<1>(pair));
                        }}
                << ")";
        });
    };

    auto res = builder(pvd);
    res << "INSERT INTO " << tbl_nm << " (" << list{col_nms, ", ", id<>}
        << ") VALUES\n"
        << list{rows, ",\n", encode};
    return res;
};

inline sql_builder insert_sql(provider& pvd,
                              const qualified_name& tbl_nm,
                              const rowset& rows)
{
    return insert_sql(pvd, tbl_nm, rows.columns, select(rows));
}

inline sql_builder drop_sql(provider& pvd, const qualified_name& tbl_nm)
{
    auto res = builder(pvd);
    res << "DROP TABLE " << tbl_nm;
    return res;
}

inline meta::column column(provider& pvd, const qualified_name& col_nm)
{
    return *find(pvd.table(qualifier(col_nm)).columns, col_nm.back());
}

}  // namespace bark::db

#endif  // BARK_DB_PROVIDER_HPP
