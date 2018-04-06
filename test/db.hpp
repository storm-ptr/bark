// Andrew Naplavkov

#ifndef BARK_TEST_DB_HPP
#define BARK_TEST_DB_HPP

#include <bark/dataset/rowset_ops.hpp>
#include <bark/db/gdal/provider.hpp>
#include <bark/db/mysql/provider.hpp>
#include <bark/db/odbc/provider.hpp>
#include <bark/db/postgres/provider.hpp>
#include <bark/db/sqlite/provider.hpp>
#include <bark/detail/random_index.hpp>
#include <bark/test/wkb_unifier.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <iostream>

inline auto random_name()
{
    std::ostringstream os;
    os << "drop_me_" << bark::random_index{10000}();
    return bark::db::id(os.str());
}

const auto& odbc_driver(std::initializer_list<bark::string_view> tokens)
{
    using namespace bark;
    static const auto Drivers = db::odbc::drivers();
    return *boost::range::find_if(Drivers, [tokens](const auto& drv) {
        return all_of(tokens, within(drv));
    });
}

inline std::vector<bark::db::provider_ptr> make_write_providers()
{
    using namespace bark::db;
    static const std::string Ip = "192.168.170.128";
    static const std::string Pwd = "E207cGYM";
    return {std::make_shared<mysql::provider>(Ip, 3306, "mysql", "root", Pwd),
            std::make_shared<odbc::provider>(
                "DRIVER=" + odbc_driver({"IBM"}) + ";UID=DB2INST1;PWD=" + Pwd +
                ";DATABASE=SAMPLE;HOSTNAME=" + Ip + ";"),
            std::make_shared<odbc::provider>(
                "DRIVER=" + odbc_driver({"MySQL", "Unicode"}) +
                ";UID=root;PWD=" + Pwd + ";DATABASE=mysql;SERVER=" + Ip +
                ";MULTI_STATEMENTS=1;"),
            std::make_shared<odbc::provider>(
                "DRIVER=" + odbc_driver({"PostgreSQL", "Unicode"}) +
                ";UID=postgres;PWD=" + Pwd + ";DATABASE=postgres;SERVER=" + Ip +
                ";"),
            std::make_shared<odbc::provider>(
                "DRIVER=" + odbc_driver({"SQL", "Server"}) +
                ";UID=sa;PWD=" + Pwd + ";DATABASE=master;SERVER=" + Ip + ";"),
            std::make_shared<postgres::provider>(
                Ip, 5432, "postgres", "postgres", Pwd),
            std::make_shared<sqlite::provider>(R"(./drop_me.sqlite)")};
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
    auto cols = column_names(tbl_from.columns);
    auto col = std::distance(tbl_from.columns.begin(),
                             find(tbl_from.columns, lr_from.back()));
    auto rows_from = select(pvd_from, tbl_from.name, cols, 1, Limit);
    unify(rows_from, col);
    std::cout << rows_from << std::endl;
    REQUIRE(!rows_from.empty());

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
        cmd->exec(insert_sql(*pvd_to, tbl_to.name, cols, rows_from));
        cmd->commit();
        cmd.reset(nullptr);
        pvd_to->refresh();
        auto rows_to = select(*pvd_to, tbl_to.name, cols, 0, Limit);
        unify(rows_to, col);
        REQUIRE(rows_from == rows_to);

        drop(*pvd_to, tbl_to.name);
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
              << geometry::wkt(pvd.extent(layer)) << std::endl;
}

#endif  // BARK_TEST_DB_HPP
