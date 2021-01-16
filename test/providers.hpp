// Andrew Naplavkov

#ifndef BARK_TEST_PROVIDERS_HPP
#define BARK_TEST_PROVIDERS_HPP

#include <bark/db/gdal/provider.hpp>
#include <bark/db/mysql/provider.hpp>
#include <bark/db/odbc/provider.hpp>
#include <bark/db/postgres/provider.hpp>
#include <bark/db/sqlite/provider.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/range/algorithm/find_if.hpp>

inline const auto& odbc_driver(std::initializer_list<std::string_view> tokens)
{
    using namespace bark;
    static const auto Drivers = db::odbc::drivers();
    auto it = boost::range::find_if(Drivers, [tokens](const auto& drv) {
        return all_of(tokens, within(drv));
    });
    if (it == std::end(Drivers))
        throw std::runtime_error(
            concat("ODBC driver not found: ", list{tokens, ", "}));
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

inline std::vector<bark::db::provider_ptr> make_providers()
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
        std::make_shared<odbc::provider>("DRIVER=" + odbc_driver({"MySQL", "Unicode"}) + ";UID=root;PWD=" BOOST_PP_STRINGIZE(BARK_TEST_DATABASE_PWD) ";DATABASE=mysql;SERVER=" BOOST_PP_STRINGIZE(BARK_TEST_MYSQL_SERVER) ";MULTI_STATEMENTS=1;SSLMODE=disabled"),
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

#endif  // BARK_TEST_PROVIDERS_HPP
