// Andrew Naplavkov

#ifndef BARK_DB_UTILITY_HPP
#define BARK_DB_UTILITY_HPP

#include <algorithm>
#include <bark/db/sql_builder.hpp>
#include <bark/geometry/as_binary.hpp>
#include <initializer_list>
#include <sstream>

namespace bark::db {

template <class ColumnNames>
auto index_name(const qualified_name& tbl, const ColumnNames& cols)
{
    std::ostringstream os;
    os << "idx_" << tbl.back() << "_" << list{cols, "_"};
    return id(os.str());
}

/// OpenGIS Document 99-049
/// @see http://www.opengeospatial.org/standards/sfs
inline bool is_ogc_type(std::string_view type)
{
    static const std::string_view Prefix = "st_";
    if (type.find(Prefix) == 0)
        type.remove_prefix(Prefix.size());
    return any_of(
        {
            "collection",          // synonym
            "curve",               // non-instantiable
            "geomcollection",      // synonym
            "geometry",            // non-instantiable
            "geometrycollection",  //
            "linestring",          //
            "multicurve",          // non-instantiable
            "multilinestring",     //
            "multipoint",          //
            "multipolygon",        //
            "multisurface",        // non-instantiable
            "point",               //
            "polygon",             //
            "surface"              // non-instantiable
        },
        equals(type));
}

inline column_type iso_type(std::string_view type, int scale)
{
    if ((within(type)("int") && !within(type)("interval")) ||
        type.find("bool") == 0)
        return column_type::Integer;
    if (any_of({"date", "time", "char", "clob", "text"}, within(type)))
        return column_type::Text;
    if (any_of({"real", "float", "double"}, within(type)))
        return column_type::Real;
    if (any_of({"decimal", "number", "numeric"}, equals(type)))
        return scale ? column_type::Real : column_type::Integer;
    if (any_of({"binary", "blob"}, within(type)))
        return column_type::Blob;
    return column_type::Invalid;
}

inline sql_syntax embeded_params(sql_syntax syntax)
{
    syntax.parameter_marker = nullptr;
    return syntax;
}

inline void ogc_projections_sql(sql_builder& bld)
{
    bld << "SELECT srid, (CASE LOWER(auth_name) WHEN " << param{"epsg"}
        << " THEN auth_srid ELSE NULL END), proj4text FROM "
           "spatial_ref_sys";
}

inline void iso_layers_sql(sql_builder& bld,
                           std::initializer_list<std::string_view> types)
{
    bld << "SELECT table_schema, table_name, column_name FROM "
           "information_schema.columns WHERE LOWER(data_type) IN ("
        << list{types, ",", [](auto type) { return param{type}; }} << ")";
}

inline void iso_columns_sql(sql_builder& bld, const qualified_name& tbl_nm)
{
    auto& scm = tbl_nm.at(-2);
    bld << "SELECT column_name, LOWER(data_type), numeric_scale FROM "
           "information_schema.columns WHERE table_schema = "
        << param{scm} << " AND table_name = " << param{tbl_nm.back()}
        << " ORDER BY ordinal_position";
}

inline void ogc_projection_sql(sql_builder& bld, const qualified_name& col_nm)
{
    auto& tbl = col_nm.at(-2);
    auto& scm = col_nm.at(-3);
    bld << "SELECT srid FROM geometry_columns WHERE ";
    if (!scm.empty())
        bld << "LOWER(f_table_schema) = LOWER(" << param{scm} << ") AND ";
    bld << "LOWER(f_table_name) = LOWER(" << param{tbl}
        << ") AND LOWER(f_geometry_column) = LOWER(" << param{col_nm.back()}
        << ")";
}

inline void ogc_extent_sql(sql_builder& bld, const qualified_name& col_nm)
{
    bld << R"(
SELECT COUNT(1),
       Min(ST_X(ST_PointN(e, 1))),
       Min(ST_Y(ST_PointN(e, 1))),
       Max(ST_X(ST_PointN(e, 3))),
       Max(ST_Y(ST_PointN(e, 3)))
FROM (SELECT ST_ExteriorRing(ST_Envelope()"
        << id(col_nm.back()) << ")) e FROM " << qualifier(col_nm) << ") t";
}

inline auto st_as_binary()
{
    return [](sql_builder& bld, std::string_view col_nm) {
        bld << "ST_AsBinary(" << id(col_nm) << ") AS " << id(col_nm);
    };
}

inline auto st_geom_from_wkb(int srid)
{
    return [srid](sql_builder& bld, variant_t val) {
        bld << "ST_GeomFromWKB(" << param{val} << ", " << srid << ")";
    };
}

inline void iso_page_clause(sql_builder& bld, size_t offset, size_t limit)
{
    bld << "OFFSET " << offset << " ROWS FETCH NEXT " << limit << " ROWS ONLY";
}

inline void limit_page_clause(sql_builder& bld, size_t offset, size_t limit)
{
    bld << "LIMIT " << limit << " OFFSET " << offset;
}

}  // namespace bark::db

#endif  // BARK_DB_UTILITY_HPP
