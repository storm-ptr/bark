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

class provider : private cacher<provider>,
                 private ddl<provider>,
                 private projection_guide<provider>,
                 public provider_impl<provider>,
                 private table_guide<provider> {
    friend cacher<provider>;
    friend ddl<provider>;
    friend projection_guide<provider>;
    friend provider_impl<provider>;
    friend table_guide<provider>;

public:
    explicit provider(const std::string& conn_str)
        : provider_impl<provider>{
              [=] { return new command(conn_str); },
              [&]() -> dialect_holder {
                  auto cmd = command(conn_str);
                  auto dbms = unicode::to_lower(cmd.dbms_name());
                  if (within(dbms)("db2"))
                      return std::make_unique<db2_dialect>();
                  else if (all_of({"microsoft", "sql", "server"}, within(dbms)))
                      return std::make_unique<mssql_dialect>();
                  else if (within(dbms)("mysql"))
                      return std::make_unique<mysql_dialect>(cmd);
                  else if (within(dbms)("postgres"))
                      return std::make_unique<postgres_dialect>();
                  else
                      throw std::runtime_error("unsupported DBMS: " + dbms);
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
