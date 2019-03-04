// Andrew Naplavkov

#ifndef BARK_TEST_DB_HPP
#define BARK_TEST_DB_HPP

#include <bark/db/gdal/provider.hpp>
#include <bark/db/mysql/provider.hpp>
#include <bark/db/odbc/provider.hpp>
#include <bark/db/postgres/provider.hpp>
#include <bark/db/sqlite/provider.hpp>
#include <bark/detail/random_index.hpp>
#include <bark/test/wkb_unifier.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <iostream>

inline auto random_name()
{
    std::ostringstream os;
    os << "drop_me_" << bark::random_index{10000}();
    return bark::db::id(os.str());
}

const auto& odbc_driver(std::initializer_list<std::string_view> tokens)
{
    using namespace bark;
    static const auto Drivers = db::odbc::drivers();
    auto it = boost::range::find_if(Drivers, [tokens](const auto& drv) {
        return all_of(tokens, within(drv));
    });
    if (it == boost::end(Drivers)) {
        std::ostringstream os;
        os << "ODBC driver not found: " << list{tokens, ", "};
        throw std::runtime_error(os.str());
    }
    return *it;
}

#if defined(BARK_TEST_DATABASE_SERVER) && defined(BARK_TEST_DATABASE_PWD)
#define BARK_TEST_DB2_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_DB2_PWD BARK_TEST_DATABASE_PWD
#define BARK_TEST_MSSQL_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_MSSQL_PWD BARK_TEST_DATABASE_PWD
#define BARK_TEST_MYSQL_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_MYSQL_PWD BARK_TEST_DATABASE_PWD
#define BARK_TEST_POSTGRES_SERVER BARK_TEST_DATABASE_SERVER
#define BARK_TEST_POSTGRES_PWD BARK_TEST_DATABASE_PWD
#endif

inline std::vector<bark::db::provider_ptr> make_write_providers()
{
    using namespace bark::db;
    return
    {
#if defined(BARK_TEST_DB2_SERVER) && defined(BARK_TEST_DB2_PWD)
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"IBM"}) + ";UID=DB2INST1;PWD=" BOOST_PP_STRINGIZE( BARK_TEST_DB2_PWD) ";DATABASE=SAMPLE;HOSTNAME=" BOOST_PP_STRINGIZE(BARK_TEST_DB2_SERVER)),
#endif

#if defined(BARK_TEST_MSSQL_SERVER) && defined(BARK_TEST_MSSQL_PWD)
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"SQL", "Server"}) + ";UID=sa;PWD=" BOOST_PP_STRINGIZE( BARK_TEST_MSSQL_PWD) ";DATABASE=master;SERVER=" BOOST_PP_STRINGIZE(BARK_TEST_MSSQL_SERVER)),
#endif

#if defined(BARK_TEST_MYSQL_SERVER) && defined(BARK_TEST_MYSQL_PWD)
        std::make_shared<mysql::provider>(BOOST_PP_STRINGIZE(BARK_TEST_MYSQL_SERVER), 3306, "mysql", "root", BOOST_PP_STRINGIZE(BARK_TEST_MYSQL_PWD)),
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"MySQL", "Unicode"}) + ";UID=root;PWD=" BOOST_PP_STRINGIZE(BARK_TEST_MYSQL_PWD) ";DATABASE=mysql;SERVER=" BOOST_PP_STRINGIZE(BARK_TEST_MYSQL_SERVER) ";MULTI_STATEMENTS=1"),
#endif

#if defined(BARK_TEST_POSTGRES_SERVER) && defined(BARK_TEST_POSTGRES_PWD)
        std::make_shared<postgres::provider>(BOOST_PP_STRINGIZE(BARK_TEST_POSTGRES_SERVER), 5432, "postgres", "postgres", BOOST_PP_STRINGIZE(BARK_TEST_POSTGRES_PWD)),
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"PostgreSQL", "Unicode"}) + ";UID=postgres;PWD=" BOOST_PP_STRINGIZE(BARK_TEST_POSTGRES_PWD) ";DATABASE=postgres;SERVER=" BOOST_PP_STRINGIZE(BARK_TEST_POSTGRES_SERVER)),
#endif

        std::make_shared<sqlite::provider>(R"(./drop_me.sqlite)")
    };
}

inline bool operator==(const bark::db::rowset& lhs, const bark::db::rowset& rhs)
{
    return std::make_tuple(lhs.columns.size(),
                           static_cast<bark::blob_view>(lhs.data)) ==
           std::make_tuple(rhs.columns.size(),
                           static_cast<bark::blob_view>(rhs.data));
}

TEST_CASE("db_geometry")
{
    using namespace bark::db;
    static constexpr size_t Limit = 10;

    boost::io::ios_flags_saver ifs(std::cout);
    gdal::provider pvd_from(R"(./data/mexico.sqlite)");
    auto lr_from = pvd_from.dir().begin()->first;
    auto tbl_from = pvd_from.table(qualifier(lr_from));
    std::cout << tbl_from << std::endl;
    REQUIRE(!tbl_from.columns.empty());
    REQUIRE(!tbl_from.indexes.empty());
    auto cols = names(tbl_from.columns);
    auto col = std::distance(tbl_from.columns.begin(),
                             find(tbl_from.columns, lr_from.back()));
    auto rows_from = select(pvd_from, tbl_from.name, cols, 1, Limit);
    unify(rows_from, col);
    std::cout << rows_from << std::endl;
    REQUIRE(!rows_from.data.empty());

    for (auto& pvd_to : make_write_providers()) {
        auto scr =
            pvd_to->script({random_name(), tbl_from.columns, tbl_from.indexes});
        REQUIRE_THROWS(pvd_to->table(scr.name));
        exec(*pvd_to, scr.sql);
        pvd_to->refresh();
        auto tbl_to = pvd_to->table(scr.name);
        REQUIRE(tbl_from == tbl_to);

        auto cmd = pvd_to->make_command();
        cmd->set_autocommit(false);
        cmd->exec(insert_sql(*pvd_to, tbl_to.name, cols, range(rows_from)));
        cmd->commit();
        pvd_to->refresh();
        auto rows_to = select(*pvd_to, tbl_to.name, cols, 0, Limit);
        unify(rows_to, col);
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
              << proj::print(pvd.projection(layer)) << '\n'
              << geometry::wkt{pvd.extent(layer)} << std::endl;
}

#endif  // BARK_TEST_DB_HPP
