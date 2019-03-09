// Andrew Naplavkov

#ifndef BARK_DB_TABLE_DEF_HPP
#define BARK_DB_TABLE_DEF_HPP

#include <bark/db/sql_builder.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <functional>
#include <string>
#include <vector>

namespace bark::db {

enum class column_type { Invalid, Blob, Geometry, Integer, Real, Text };

enum class index_type { Invalid, Primary, Secondary };

/// Converts to a well-known representation, e.g. ST_AsBinary
using column_decoder = std::function<void(sql_builder&, std::string_view name)>;

/// Constructs a value using its well-known representation, e.g. ST_GeomFromWKB
using column_encoder = std::function<void(sql_builder&, variant_t val)>;

using rtree =
    boost::geometry::index::rtree<geometry::box,
                                  boost::geometry::index::quadratic<16>>;

struct column_def {
    std::string name;
    column_type type = column_type::Invalid;

    column_decoder decoder = [](sql_builder& bld, std::string_view name) {
        bld << id(name);
    };

    column_encoder encoder = [](sql_builder& bld, variant_t val) {
        bld << param{val};
    };

    std::string projection;
    rtree tiles;
};

struct index_def {
    index_type type = index_type::Invalid;
    std::vector<std::string> columns;
};

struct table_def {
    qualified_name name;
    std::vector<column_def> columns;
    std::vector<index_def> indexes;
};

}  // namespace bark::db

#endif  // BARK_DB_TABLE_DEF_HPP
