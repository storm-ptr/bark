// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_PROVIDER_HPP
#define BARK_DB_MYSQL_PROVIDER_HPP

#include <bark/db/detail/cacher.hpp>
#include <bark/db/detail/ddl.hpp>
#include <bark/db/detail/mysql_dialect.hpp>
#include <bark/db/detail/projection_guide.hpp>
#include <bark/db/detail/provider_impl.hpp>
#include <bark/db/detail/table_guide.hpp>
#include <bark/db/mysql/command.hpp>
#include <memory>
#include <string>

namespace bark {
namespace db {
namespace mysql {

class provider : private db::detail::cacher<db::mysql::provider>,
                 private db::detail::ddl<db::mysql::provider>,
                 private db::detail::projection_guide<db::mysql::provider>,
                 public db::detail::provider_impl<db::mysql::provider>,
                 private db::detail::table_guide<db::mysql::provider> {
    friend db::detail::cacher<db::mysql::provider>;
    friend db::detail::ddl<db::mysql::provider>;
    friend db::detail::projection_guide<db::mysql::provider>;
    friend db::detail::provider_impl<db::mysql::provider>;
    friend db::detail::table_guide<db::mysql::provider>;

public:
    provider(std::string host,
             int port,
             std::string db,
             std::string usr,
             std::string pwd)
        : db::detail::provider_impl<db::mysql::provider>{
              [=] { return new db::mysql::command(host, port, db, usr, pwd); },
              std::make_unique<db::detail::mysql_dialect>()}
    {
    }
};

}  // namespace mysql
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_MYSQL_PROVIDER_HPP
