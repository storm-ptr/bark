// Andrew Naplavkov

#ifndef BARK_TEST_DB_HPP
#define BARK_TEST_DB_HPP

#include <bark/test/providers.hpp>
#include <bark/test/simplify_geometry.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <iostream>

namespace bark::db {

inline void simplify_geometry_column(const rowset& rows, std::string_view col)
{
    auto it = std::find(rows.columns.begin(), rows.columns.end(), col);
    auto offset = std::distance(rows.columns.begin(), it);
    for_each_blob(select(rows), offset, simplify_geometry{});
}

inline bool operator==(const rowset& lhs, const rowset& rhs)
{
    return std::is_permutation(lhs.columns.begin(),
                               lhs.columns.end(),
                               rhs.columns.begin(),
                               rhs.columns.end()) &&
           select(lhs) == select(lhs.columns, rhs);
}

namespace meta {

inline bool operator==(const column& lhs, const column& rhs)
{
    return std::tie(lhs.name, lhs.type, lhs.projection) ==
           std::tie(rhs.name, rhs.type, rhs.projection);
}

inline bool operator==(const index& lhs, const index& rhs)
{
    return lhs.type == rhs.type &&
           boost::range::equal(lhs.columns, rhs.columns);
}

inline bool operator==(const table& lhs, const table& rhs)
{
    /// the table name is not compared (structure only)
    return std::is_permutation(lhs.columns.begin(),
                               lhs.columns.end(),
                               rhs.columns.begin(),
                               rhs.columns.end()) &&
           std::is_permutation(lhs.indexes.begin(),
                               lhs.indexes.end(),
                               rhs.indexes.begin(),
                               rhs.indexes.end());
}

}  // namespace meta

}  // namespace bark::db

TEST_CASE("db_insert")
{
    using namespace bark;
    using namespace bark::db;
    auto ifs = boost::io::ios_flags_saver{std::cout};
    auto pvd_src = gdal::provider{"./data/mexico.sqlite"};
    auto lr_src = pvd_src.dir().begin()->first;
    auto tbl_src = pvd_src.table(qualifier(lr_src));
    std::cout << tbl_src << std::endl;
    REQUIRE(!tbl_src.columns.empty());
    REQUIRE(!tbl_src.indexes.empty());
    auto rows_src = fetch_all(pvd_src, select_sql(pvd_src, tbl_src.name, 3, 5));
    std::cout << rows_src << std::endl;
    REQUIRE(!rows_src.data.empty());
    simplify_geometry_column(rows_src, lr_src.back());
    for (auto& pvd : make_providers()) {
        auto [tbl_nm, ddl] =
            pvd->ddl({id(concat("drop_me_", random_index{10000}())),
                      tbl_src.columns,
                      tbl_src.indexes});
        REQUIRE_THROWS(pvd->table(tbl_nm));
        exec(*pvd, ddl);
        pvd->refresh();
        REQUIRE(tbl_src == pvd->table(tbl_nm));
        exec(*pvd, insert_sql(*pvd, tbl_nm, rows_src));
        pvd->refresh();
        auto rows = fetch_all(*pvd, select_sql(*pvd, tbl_nm, 0, 100500));
        simplify_geometry_column(rows, lr_src.back());
        REQUIRE(rows_src == rows);
        exec(*pvd, drop_sql(*pvd, tbl_nm));
        pvd->refresh();
        REQUIRE_THROWS(pvd->table(tbl_nm));
    }
}

#endif  // BARK_TEST_DB_HPP
