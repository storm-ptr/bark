// Andrew Naplavkov

#ifndef BARK_DB_TABLE_DEF_HPP
#define BARK_DB_TABLE_DEF_HPP

#include <bark/dataset/variant_view.hpp>
#include <bark/db/qualified_name.hpp>
#include <bark/db/sql_builder.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/utility/string_view.hpp>
#include <functional>
#include <string>
#include <vector>

namespace bark {
namespace db {

enum class column_type { Invalid, Blob, Geometry, Integer, Real, Text };

enum class index_type { Invalid, Primary, Secondary };

using column_decoder =
    std::function<void(sql_builder&, boost::string_view col_nm)>;

using column_encoder =
    std::function<void(sql_builder&, dataset::variant_view val)>;

using rtree =
    boost::geometry::index::rtree<geometry::box,
                                  boost::geometry::index::quadratic<16>>;

struct column_def {
    std::string name;
    column_type type = column_type::Invalid;
    std::string projection;

    column_decoder decoder = [](sql_builder& bld, boost::string_view col_nm) {
        bld << id(col_nm);
    };

    column_encoder encoder = [](sql_builder& bld, dataset::variant_view val) {
        bld << param(val);
    };

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

}  // namespace db
}  // namespace bark

#endif  // BARK_DB_TABLE_DEF_HPP
