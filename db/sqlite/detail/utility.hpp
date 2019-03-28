// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_UTILITY_HPP
#define BARK_DB_SQLITE_UTILITY_HPP

#include <memory>
#include <stdexcept>
#include <string>

#include <sqlite3.h>
// strictly ordered
#include <spatialite.h>

namespace bark::db::sqlite {

struct connection_deleter {
    void operator()(sqlite3* p) const { sqlite3_close_v2(p); }
};

using connection_holder = std::unique_ptr<sqlite3, connection_deleter>;

struct statement_deleter {
    void operator()(sqlite3_stmt* p) const { sqlite3_finalize(p); }
};

using statement_holder = std::unique_ptr<sqlite3_stmt, statement_deleter>;

struct spatialite_deleter {
    void operator()(void* p) const { spatialite_cleanup_ex(p); }
};

using spatialite_holder = std::unique_ptr<void, spatialite_deleter>;

inline void check(const connection_holder& con, int r)
{
    switch (r) {
        case SQLITE_DONE:
        case SQLITE_OK:
        case SQLITE_ROW:
            break;
        default: {
            std::string msg;
            if (con)
                msg = sqlite3_errmsg(con.get());
            if (msg.empty())
                msg = "SQLite error";
            throw std::runtime_error(msg);
        }
    }
}

}  // namespace bark::db::sqlite

#endif  // BARK_DB_SQLITE_UTILITY_HPP
