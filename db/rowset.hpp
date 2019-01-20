// Andrew Naplavkov

#ifndef BARK_DB_ROWSET_HPP
#define BARK_DB_ROWSET_HPP

#include <bark/blob.hpp>
#include <cstdint>
#include <string>

namespace bark::db {

using variant_t =
    std::variant<std::monostate, int64_t, double, std::string_view, blob_view>;

using row_t = std::vector<variant_t>;

struct variant_istream {
    blob_view data;
};

struct variant_ostream {
    blob data;
};

struct rowset {
    std::vector<std::string> columns;
    blob data;
};

}  // namespace bark::db

#endif  // BARK_DB_ROWSET_HPP
