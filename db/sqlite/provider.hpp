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

namespace bark {
namespace db {
namespace sqlite {

class provider : private db::detail::cacher<db::sqlite::provider>,
                 private db::detail::ddl<db::sqlite::provider>,
                 private db::detail::projection_guide<db::sqlite::provider>,
                 public db::detail::provider_impl<db::sqlite::provider>,
                 private db::sqlite::detail::table_guide<db::sqlite::provider> {
    friend db::detail::cacher<db::sqlite::provider>;
    friend db::detail::ddl<db::sqlite::provider>;
    friend db::detail::projection_guide<db::sqlite::provider>;
    friend db::detail::provider_impl<db::sqlite::provider>;
    friend db::sqlite::detail::table_guide<db::sqlite::provider>;

public:
    provider(std::string file)
        : db::detail::provider_impl<db::sqlite::provider>{
              [=] { return new db::sqlite::command(file); },
              std::make_unique<db::sqlite::detail::dialect>()}
    {
        try {
            exec(*this, "SELECT InitSpatialMetaData(1)");
        }
        catch (const std::exception&) {
            // already init
        }
    }
};

}  // namespace sqlite
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SQLITE_PROVIDER_HPP
