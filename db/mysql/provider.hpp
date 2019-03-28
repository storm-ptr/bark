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

namespace bark::db::mysql {

class provider : private cacher<mysql::provider>,
                 private ddl<mysql::provider>,
                 private projection_guide<mysql::provider>,
                 public provider_impl<mysql::provider>,
                 private table_guide<mysql::provider> {
    friend cacher<mysql::provider>;
    friend ddl<mysql::provider>;
    friend projection_guide<mysql::provider>;
    friend provider_impl<mysql::provider>;
    friend table_guide<mysql::provider>;

public:
    provider(std::string host,
             int port,
             std::string db,
             std::string usr,
             std::string pwd)
        : provider_impl<mysql::provider>{
              [=] { return new db::mysql::command(host, port, db, usr, pwd); },
              std::make_unique<mysql_dialect>()}
    {
    }
};

}  // namespace bark::db::mysql

#endif  // BARK_DB_MYSQL_PROVIDER_HPP
