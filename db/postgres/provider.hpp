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

namespace bark::db::postgres {

class provider : private cacher<postgres::provider>,
                 private ddl<postgres::provider>,
                 private projection_guide<postgres::provider>,
                 public provider_impl<postgres::provider>,
                 private table_guide<postgres::provider> {
    friend cacher<postgres::provider>;
    friend ddl<postgres::provider>;
    friend projection_guide<postgres::provider>;
    friend provider_impl<postgres::provider>;
    friend table_guide<postgres::provider>;

public:
    provider(std::string host,
             int port,
             std::string db,
             std::string usr,
             std::string pwd)
        : provider_impl<postgres::provider>{
              [=] {
                  return new db::postgres::command(host, port, db, usr, pwd);
              },
              std::make_unique<postgres_dialect>()}
    {
    }
};

}  // namespace bark::db::postgres

#endif  // BARK_DB_POSTGRES_PROVIDER_HPP
