// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_PROVIDER_HPP
#define BARK_DB_POSTGRES_PROVIDER_HPP

#include <bark/db/detail/cacher.hpp>
#include <bark/db/detail/ddl.hpp>
#include <bark/db/detail/postgres_dialect.hpp>
#include <bark/db/detail/projection_guide.hpp>
#include <bark/db/detail/provider_impl.hpp>
#include <bark/db/detail/table_guide.hpp>
#include <bark/db/postgres/command.hpp>
#include <memory>
#include <string>

namespace bark {
namespace db {
namespace postgres {

class provider : private db::detail::cacher<db::postgres::provider>,
                 private db::detail::ddl<db::postgres::provider>,
                 private db::detail::projection_guide<db::postgres::provider>,
                 public db::detail::provider_impl<db::postgres::provider>,
                 private db::detail::table_guide<db::postgres::provider> {
    friend db::detail::cacher<db::postgres::provider>;
    friend db::detail::ddl<db::postgres::provider>;
    friend db::detail::projection_guide<db::postgres::provider>;
    friend db::detail::provider_impl<db::postgres::provider>;
    friend db::detail::table_guide<db::postgres::provider>;

public:
    provider(std::string host,
             int port,
             std::string db,
             std::string usr,
             std::string pwd)
        : db::detail::provider_impl<db::postgres::provider>{
              [=] {
                  return new db::postgres::command(host, port, db, usr, pwd);
              },
              std::make_unique<db::detail::postgres_dialect>()}
    {
    }
};

}  // namespace postgres
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_POSTGRES_PROVIDER_HPP
