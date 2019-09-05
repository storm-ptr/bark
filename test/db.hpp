// Andrew Naplavkov

#ifndef BARK_TEST_DB_HPP
#define BARK_TEST_DB_HPP

#include <bark/db/gdal/provider.hpp>
#include <bark/db/mysql/provider.hpp>
#include <bark/db/odbc/provider.hpp>
#include <bark/db/postgres/provider.hpp>
#include <bark/db/sqlite/provider.hpp>
#include <bark/test/wkb_unifier.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <iostream>

namespace bark::db {

inline bool operator==(const rowset& lhs, const rowset& rhs)
{
    return std::make_tuple(lhs.columns.size(),
                           static_cast<blob_view>(lhs.data)) ==
           std::make_tuple(rhs.columns.size(),
                           static_cast<blob_view>(rhs.data));
}

inline bool operator==(const column_def& lhs, const column_def& rhs)
{
    return unicode::case_insensitive_equal_to{}(lhs.name, rhs.name) &&
           lhs.type == rhs.type && lhs.projection == rhs.projection;
}

inline bool operator==(const index_def& lhs, const index_def& rhs)
{
    return lhs.type == rhs.type &&
           boost::range::equal(
               lhs.columns, rhs.columns, unicode::case_insensitive_equal_to{});
}

inline bool operator==(const table_def& lhs, const table_def& rhs)
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

}  // namespace bark::db

inline auto random_name()
{
    std::ostringstream os;
    os << "drop_me_" << bark::random_index{10000}();
    return bark::db::id(os.str());
}

inline const auto& odbc_driver(std::initializer_list<std::string_view> tokens)
{
    using namespace bark;
    static const auto Drivers = db::odbc::drivers();
    auto it = boost::range::find_if(Drivers, [tokens](const auto& drv) {
        return all_of(tokens, within(drv));
    });
    if (it == std::end(Drivers)) {
        std::ostringstream os;
        os << "ODBC driver not found: " << list{tokens, ", "};
        throw std::runtime_error(os.str());
    }
    return *it;
}

#if defined(BARK_TEST_DATABASE_SERVER)
#define BARK_TEST_MYSQL_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_ODBC_DB2_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_ODBC_MSSQL_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_ODBC_MYSQL_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_ODBC_POSTGRES_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_POSTGRES_SERVER BARK_TEST_DATABASE_SERVER
#endif

inline std::vector<bark::db::provider_ptr> make_write_providers()
{
    using namespace bark::db;
    return
    {
#if defined(BARK_TEST_MYSQL_SERVER) && defined(BARK_TEST_DATABASE_PWD)
        std::make_shared<mysql::provider>(BOOST_PP_STRINGIZE(BARK_TEST_MYSQL_SERVER), 3306, "mysql", "root", BOOST_PP_STRINGIZE(BARK_TEST_DATABASE_PWD)),
#endif
#if defined(BARK_TEST_ODBC_DB2_SERVER) && defined(BARK_TEST_DATABASE_PWD)
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"IBM"}) + ";UID=DB2INST1;PWD=" BOOST_PP_STRINGIZE(BARK_TEST_DATABASE_PWD) ";DATABASE=SAMPLE;HOSTNAME=" BOOST_PP_STRINGIZE(BARK_TEST_ODBC_DB2_SERVER)),
#endif
#if defined(BARK_TEST_ODBC_MSSQL_SERVER) && defined(BARK_TEST_DATABASE_PWD)
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"SQL", "Server"}) + ";UID=sa;PWD=" BOOST_PP_STRINGIZE(BARK_TEST_DATABASE_PWD) ";DATABASE=master;SERVER=" BOOST_PP_STRINGIZE(BARK_TEST_ODBC_MSSQL_SERVER)),
#endif
#if defined(BARK_TEST_ODBC_MYSQL_SERVER) && defined(BARK_TEST_DATABASE_PWD)
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"MySQL", "Unicode"}) + ";UID=root;PWD=" BOOST_PP_STRINGIZE(BARK_TEST_DATABASE_PWD) ";DATABASE=mysql;SERVER=" BOOST_PP_STRINGIZE(BARK_TEST_MYSQL_SERVER) ";MULTI_STATEMENTS=1"),
#endif
#if defined(BARK_TEST_ODBC_POSTGRES_SERVER) && defined(BARK_TEST_DATABASE_PWD)
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"PostgreSQL", "Unicode"}) + ";UID=postgres;PWD=" BOOST_PP_STRINGIZE(BARK_TEST_DATABASE_PWD) ";DATABASE=postgres;SERVER=" BOOST_PP_STRINGIZE(BARK_TEST_POSTGRES_SERVER)),
#endif
#if defined(BARK_TEST_POSTGRES_SERVER) && defined(BARK_TEST_DATABASE_PWD)
        std::make_shared<postgres::provider>(BOOST_PP_STRINGIZE(BARK_TEST_POSTGRES_SERVER), 5432, "postgres", "postgres", BOOST_PP_STRINGIZE(BARK_TEST_DATABASE_PWD)),
#endif
        std::make_shared<sqlite::provider>(R"(./drop_me.sqlite)")
    };
}

TEST_CASE("db_geometry")
{
    using namespace bark::db;
    static constexpr size_t Limit = 10;

    boost::io::ios_flags_saver ifs(std::cout);
    gdal::provider pvd_from("./data/mexico.sqlite");
    auto lr_from = pvd_from.dir().begin()->first;
    auto tbl_from = pvd_from.table(qualifier(lr_from));
    std::cout << tbl_from << std::endl;
    REQUIRE(!tbl_from.columns.empty());
    REQUIRE(!tbl_from.indexes.empty());
    auto cols = names(tbl_from.columns);
    auto col = std::distance(tbl_from.columns.begin(),
                             find(tbl_from.columns, lr_from.back()));
    auto rowset_from =
        fetch_all(pvd_from, select_sql(pvd_from, tbl_from.name, 1, Limit));
    std::cout << rowset_from << std::endl;
    REQUIRE(!rowset_from.data.empty());
    auto rows_from = select(cols, rowset_from);
    bark::db::for_each_blob(rows_from, col, wkb_unifier{});
    for (auto& pvd_to : make_write_providers()) {
        auto [name, sql] =
            pvd_to->script({random_name(), tbl_from.columns, tbl_from.indexes});
        REQUIRE_THROWS(pvd_to->table(name));
        exec(*pvd_to, sql);
        pvd_to->refresh();
        auto tbl_to = pvd_to->table(name);
        REQUIRE(tbl_from == tbl_to);

        auto cmd = pvd_to->make_command();
        cmd->set_autocommit(false);
        cmd->exec(insert_sql(*pvd_to, tbl_to.name, cols, rows_from));
        cmd->commit();
        pvd_to->refresh();
        auto rowset_from =
            fetch_all(*pvd_to, select_sql(*pvd_to, tbl_to.name, 0, Limit));
        auto rows_to = select(cols, rowset_from);
        bark::db::for_each_blob(rows_to, col, wkb_unifier{});
        REQUIRE(rows_from == rows_to);

        cmd->exec(drop_sql(*pvd_to, tbl_to.name));
        cmd->commit();
        pvd_to->refresh();
        REQUIRE_THROWS(pvd_to->table(tbl_to.name));
    }
}

TEST_CASE("gdal_raster")
{
    using namespace bark;
    using namespace bark::db;

    gdal::provider pvd(R"(./data/albers27.tif)");
    auto reg = pvd.dir();
    REQUIRE(!reg.empty());
    auto layer = reg.begin()->first;
    std::cout << layer << '\n'
              << proj::abbreviation(pvd.projection(layer)) << '\n'
              << geometry::wkt{pvd.extent(layer)} << std::endl;
}

#endif  // BARK_TEST_DB_HPP
