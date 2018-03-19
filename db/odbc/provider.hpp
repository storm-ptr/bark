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
#include <bark/unicode.hpp>
#include <boost/utility/string_view.hpp>
#include <memory>
#include <stdexcept>
#include <string>

namespace bark {
namespace db {
namespace odbc {

class provider : private db::detail::cacher<db::odbc::provider>,
                 private db::detail::ddl<db::odbc::provider>,
                 private db::detail::projection_guide<db::odbc::provider>,
                 public db::detail::provider_impl<db::odbc::provider>,
                 private db::detail::table_guide<db::odbc::provider> {
    friend db::detail::cacher<db::odbc::provider>;
    friend db::detail::ddl<db::odbc::provider>;
    friend db::detail::projection_guide<db::odbc::provider>;
    friend db::detail::provider_impl<db::odbc::provider>;
    friend db::detail::table_guide<db::odbc::provider>;

public:
    explicit provider(std::string conn_str)
        : db::detail::provider_impl<db::odbc::provider>{
              [=] { return new db::odbc::command(conn_str); },
              [=]() -> db::detail::dialect_holder {
                  auto dbms_lcase = unicode::to_lower(
                      db::odbc::command(conn_str).dbms_name());
                  if (within(dbms_lcase)("db2"))
                      return std::make_unique<db::detail::db2_dialect>();
                  else if (all_of({"microsoft", "sql", "server"},
                                  within(dbms_lcase)))
                      return std::make_unique<db::detail::mssql_dialect>();
                  else if (within(dbms_lcase)("mysql"))
                      return std::make_unique<db::detail::mysql_dialect>();
                  else if (within(dbms_lcase)("postgres"))
                      return std::make_unique<db::detail::postgres_dialect>();
                  else
                      throw std::runtime_error("unsupported DBMS: " +
                                               dbms_lcase);
              }()}
    {
    }
};

inline auto drivers()
{
    using namespace detail;
    std::vector<std::string> res;
    env_holder env(alloc_handle(env_holder{}, SQL_HANDLE_ENV));
    set_attr(env, SQL_ATTR_ODBC_VERSION, SQL_OV_ODBC3);
    buffer<SQL_MAX_MESSAGE_LENGTH> desc, attr;
    while (SQL_SUCCEEDED(SQLDriversW(env.get(),
                                     SQL_FETCH_NEXT,
                                     desc.data,
                                     desc.max_size,
                                     &desc.len,
                                     attr.data,
                                     attr.max_size,
                                     &attr.len)))
        res.push_back(desc.to_string());
    return res;
}

}  // namespace odbc
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_ODBC_PROVIDER_HPP
