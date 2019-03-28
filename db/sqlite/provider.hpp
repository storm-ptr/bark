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

class provider : private cacher<sqlite::provider>,
                 private ddl<sqlite::provider>,
                 private projection_guide<sqlite::provider>,
                 public provider_impl<sqlite::provider>,
                 private sqlite::table_guide<sqlite::provider> {
    friend cacher<sqlite::provider>;
    friend ddl<sqlite::provider>;
    friend projection_guide<sqlite::provider>;
    friend provider_impl<sqlite::provider>;
    friend sqlite::table_guide<sqlite::provider>;

public:
    provider(std::string file)
        : provider_impl<sqlite::provider>{
              [=] { return new db::sqlite::command(file); },
              std::make_unique<sqlite::dialect>()}
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
