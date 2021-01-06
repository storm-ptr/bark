// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_PROVIDER_HPP
#define BARK_DB_MYSQL_PROVIDER_HPP

#include <bark/db/detail/cacher.hpp>
#include <bark/db/detail/ddl_guide.hpp>
#include <bark/db/detail/mysql_dialect.hpp>
#include <bark/db/detail/mysql_old_dialect.hpp>
#include <bark/db/detail/projection_guide.hpp>
#include <bark/db/detail/provider_impl.hpp>
#include <bark/db/detail/table_guide.hpp>
#include <bark/db/mysql/command.hpp>
#include <memory>
#include <string>

namespace bark::db::mysql {

class provider : private cacher<provider>,
                 private ddl_guide<provider>,
                 private projection_guide<provider>,
                 public provider_impl<provider>,
                 private table_guide<provider> {
    friend cacher<provider>;
    friend ddl_guide<provider>;
    friend projection_guide<provider>;
    friend provider_impl<provider>;
    friend table_guide<provider>;

public:
    provider(std::string host,
             int port,
             std::string db,
             std::string usr,
             std::string pwd)
        : provider_impl<provider>{
              [=] { return new command(host, port, db, usr, pwd); },
              [&]() -> dialect_holder {
                  auto cmd = command(host, port, db, usr, pwd);
                  exec(cmd, "SELECT version()");
                  if (std::stoi(fetch_or(cmd, std::string{"8"}).data()) < 8)
                      return std::make_unique<mysql_old_dialect>();
                  else
                      return std::make_unique<mysql_dialect>();
              }()}
    {
    }
};

}  // namespace bark::db::mysql

#endif  // BARK_DB_MYSQL_PROVIDER_HPP
