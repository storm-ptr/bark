// Andrew Naplavkov

#ifndef BARK_DB_ODBC_PROVIDER_HPP
#define BARK_DB_ODBC_PROVIDER_HPP

#include <bark/db/detail/cacher.hpp>
#include <bark/db/detail/db2_dialect.hpp>
#include <bark/db/detail/ddl.hpp>
#include <bark/db/detail/mssql_dialect.hpp>
#include <bark/db/detail/mysql_dialect.hpp>
#include <bark/db/detail/postgres_dialect.hpp>
#include <bark/db/detail/projection_guide.hpp>
#include <bark/db/detail/provider_impl.hpp>
#include <bark/db/detail/table_guide.hpp>
#include <bark/db/odbc/command.hpp>
#include <bark/detail/unicode.hpp>
#include <memory>
#include <stdexcept>
#include <string>

namespace bark::db::odbc {

class provider : private cacher<odbc::provider>,
                 private ddl<odbc::provider>,
                 private projection_guide<odbc::provider>,
                 public provider_impl<odbc::provider>,
                 private table_guide<odbc::provider> {
    friend cacher<odbc::provider>;
    friend ddl<odbc::provider>;
    friend projection_guide<odbc::provider>;
    friend provider_impl<odbc::provider>;
    friend table_guide<odbc::provider>;

public:
    explicit provider(std::string conn_str)
        : provider_impl<odbc::provider>{
              [=] { return new db::odbc::command(conn_str); },
              [=]() -> dialect_holder {
                  auto dbms_lcase = unicode::to_lower(
                      db::odbc::command(conn_str).dbms_name());
                  if (within(dbms_lcase)("db2"))
                      return std::make_unique<db2_dialect>();
                  else if (all_of({"microsoft", "sql", "server"},
                                  within(dbms_lcase)))
                      return std::make_unique<mssql_dialect>();
                  else if (within(dbms_lcase)("mysql"))
                      return std::make_unique<mysql_dialect>();
                  else if (within(dbms_lcase)("postgres"))
                      return std::make_unique<postgres_dialect>();
                  else
                      throw std::runtime_error("unsupported DBMS: " +
                                               dbms_lcase);
              }()}
    {
    }
};

inline auto drivers()
{
    std::vector<std::string> res;
    env_holder env(alloc_handle(env_holder{}, SQL_HANDLE_ENV));
    set_attr(env, SQL_ATTR_ODBC_VERSION, SQL_OV_ODBC3);
    buffer<SQL_MAX_MESSAGE_LENGTH> desc, attr;
    while (SQL_SUCCEEDED(SQLDriversW(env.get(),
                                     SQL_FETCH_NEXT,
                                     desc.data(),
                                     desc.max_size(),
                                     0,
                                     attr.data(),
                                     attr.max_size(),
                                     0)))
        res.push_back(desc.to_string());
    return res;
}

}  // namespace bark::db::odbc

#endif  // BARK_DB_ODBC_PROVIDER_HPP
