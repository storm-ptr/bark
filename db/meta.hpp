// Andrew Naplavkov

#ifndef BARK_DB_META_HPP
#define BARK_DB_META_HPP

#include <bark/db/sql_builder.hpp>
#include <bark/geometry/geometry.hpp>
#include <bark/proj/epsg.hpp>
#include <functional>
#include <ostream>
#include <string>
#include <vector>

namespace bark::db::meta {

enum class column_type { Invalid, Blob, Geometry, Integer, Real, Text };

enum class index_type { Invalid, Primary, Secondary };

enum class layer_type { Invalid, Geometry, Raster };

/// Describes column
struct column {
    std::string name;
    column_type type = column_type::Invalid;
    std::string projection;  ///< PROJ.4 string for the spatial reference system
    geometry::box_rtree tiles;  ///< balanced data grid

    sql_decoder decoder

        = [](sql_builder& bld, std::string_view name) { bld << id(name); };

    sql_encoder encoder

        = [](sql_builder& bld, variant_t var) { bld << param{var}; };
};

/// Describes index
struct index {
    index_type type = index_type::Invalid;
    std::vector<std::string> columns;
};

/// Describes table
struct table {
    qualified_name name;
    std::vector<column> columns;
    std::vector<index> indexes;
};

inline std::ostream& operator<<(std::ostream& os, const column& col)
{
    static const char* Types[] = {
        "INVALID", "BLOB", "GEOMETRY", "INTEGER", "REAL", "TEXT"};
    os << Types[(size_t)col.type] << " " << col.name;
    if (col.type == column_type::Geometry) {
        std::vector<std::string> opts;
        if (!col.projection.empty())
            opts.push_back(proj::abbreviation(col.projection));
        if (!col.tiles.empty())
            opts.push_back("BOX" +
                           boost::lexical_cast<std::string>(
                               boost::geometry::dsv(bounds(col.tiles))));
        if (!opts.empty())
            os << " (" << list{opts, ", "} << ")";
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const index& idx)
{
    static const char* Types[] = {"INVALID", "PRIMARY", "SECONDARY"};
    return os << Types[(size_t)idx.type] << " INDEX ("
              << list{idx.columns, ", "} << ")";
}

inline std::ostream& operator<<(std::ostream& os, const table& tbl)
{
    os << "TABLE " << tbl.name << " (\n\t" << list{tbl.columns, ",\n\t"};
    if (!tbl.indexes.empty())
        os << ",\n\t" << list{tbl.indexes, ",\n\t"};
    return os << "\n)";
}

}  // namespace bark::db::meta

#endif  // BARK_DB_META_HPP
