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
    provider(std::string host,
             int port,
             std::string db,
             std::string usr,
             std::string pwd)
        : provider_impl<provider>{
              [=] { return new command(host, port, db, usr, pwd); },
              std::make_unique<postgres_dialect>()}
    {
    }
};

}  // namespace bark::db::postgres

#endif  // BARK_DB_POSTGRES_PROVIDER_HPP
