// Andrew Naplavkov

#ifndef BARK_DB_TABLE_DEF_HPP
#define BARK_DB_TABLE_DEF_HPP

#include <bark/db/sql_builder.hpp>
#include <bark/geometry/as_text.hpp>
#include <bark/geometry/geometry.hpp>
#include <bark/proj/epsg.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <functional>
#include <ostream>
#include <string>
#include <vector>

namespace bark::db {

enum class column_type { Invalid, Blob, Geometry, Integer, Real, Text };

enum class index_type { Invalid, Primary, Secondary };

/// Converts the well-known binary representation of the geometry
using column_decoder = std::function<void(sql_builder&, std::string_view name)>;

/// Creates a geometry instance from a well-known binary representation
using column_encoder = std::function<void(sql_builder&, variant_t var)>;

/// Spatial index to store values of @ref geometry::box
using rtree =
    boost::geometry::index::rtree<geometry::box,
                                  boost::geometry::index::quadratic<16>>;

/// Describes column
struct column_def {
    std::string name;
    column_type type = column_type::Invalid;
    std::string projection;  ///< PROJ.4 string for the spatial reference system
    rtree tiles;             ///< balanced data grid

    column_decoder decoder

        = [](sql_builder& bld, std::string_view name) { bld << id(name); };

    column_encoder encoder

        = [](sql_builder& bld, variant_t var) { bld << param{var}; };
};

/// Describes index
struct index_def {
    index_type type = index_type::Invalid;
    std::vector<std::string> columns;
};

/// Describes table
struct table_def {
    qualified_name name;
    std::vector<column_def> columns;
    std::vector<index_def> indexes;
};

inline std::ostream& operator<<(std::ostream& os, const column_def& col)
{
    static const char* Types[] = {
        "INVALID", "BLOB", "GEOMETRY", "INTEGER", "REAL", "TEXT"};
    os << Types[(size_t)col.type] << " " << col.name;
    if (col.type == column_type::Geometry) {
        std::vector<std::string> opts;
        if (!col.projection.empty())
            opts.push_back(proj::abbreviation(col.projection));
        if (!col.tiles.empty())
            opts.push_back(geometry::as_text(bounds(col.tiles)));
        if (!opts.empty())
            os << " (" << list{opts, ", "} << ")";
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const index_def& idx)
{
    static const char* Types[] = {"INVALID", "PRIMARY", "SECONDARY"};
    return os << Types[(size_t)idx.type] << " INDEX ("
              << list{idx.columns, ", "} << ")";
}

inline std::ostream& operator<<(std::ostream& os, const table_def& tbl)
{
    os << "TABLE " << tbl.name << " (\n\t" << list{tbl.columns, ",\n\t"};
    if (!tbl.indexes.empty())
        os << ",\n\t" << list{tbl.indexes, ",\n\t"};
    return os << "\n)";
}

}  // namespace bark::db

#endif  // BARK_DB_TABLE_DEF_HPP
