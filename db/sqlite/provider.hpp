// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_PROVIDER_HPP
#define BARK_DB_SQLITE_PROVIDER_HPP

#include <bark/db/detail/cacher.hpp>
#include <bark/db/detail/ddl.hpp>
#include <bark/db/detail/projection_guide.hpp>
#include <bark/db/detail/provider_impl.hpp>
#include <bark/db/sqlite/command.hpp>
#include <bark/db/sqlite/detail/dialect.hpp>
#include <bark/db/sqlite/detail/table_guide.hpp>
#include <exception>
#include <memory>
#include <string>

namespace bark::db::sqlite {

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
    provider(const std::string& file)
        : provider_impl<provider>{[=] { return new command(file); },
                                  std::make_unique<dialect>()}
    {
        try {
            exec(*this, "SELECT InitSpatialMetaData(1)");
        }
        catch (const std::exception&) {
            // already init
        }
    }
};

}  // namespace bark::db::sqlite

#endif  // BARK_DB_SQLITE_PROVIDER_HPP
