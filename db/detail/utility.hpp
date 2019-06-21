// Andrew Naplavkov

#ifndef BARK_DB_UTILITY_HPP
#define BARK_DB_UTILITY_HPP

#include <algorithm>
#include <bark/db/sql_builder.hpp>
#include <bark/db/table_def.hpp>
#include <bark/detail/grid.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <cmath>
#include <initializer_list>
#include <sstream>

namespace bark::db {

inline rtree make_tiles(size_t count, geometry::box ext)
{
    constexpr size_t RowsPerTile = 2000;
    rtree res;
    if (count) {
        auto side = size_t(ceil(sqrt(count / double(RowsPerTile))));
        grid gr(ext, side + 1, side + 1);
        for (size_t row = 1; row < gr.rows(); ++row)
            for (size_t col = 1; col < gr.cols(); ++col)
                res.insert({{gr.x(row - 1, col - 1), gr.y(row - 1, col - 1)},
                            {gr.x(row, col), gr.y(row, col)}});
    }
    return res;
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
           "information_schema.columns WHERE DATA_TYPE IN ("
        << list{types, ",", [](auto type) { return param{type}; }} << ")";
}

inline void iso_columns_sql(sql_builder& bld, const qualified_name& tbl_nm)
{
    auto& scm = tbl_nm.at(-2);
    bld << "SELECT column_name, data_type, numeric_scale FROM "
           "information_schema.columns WHERE table_schema = "
        << param{scm} << " AND table_name = " << param{tbl_nm.back()}
        << " ORDER BY ordinal_position";
}

/**
 * OpenGIS Document 99-049
 * @see http://www.opengeospatial.org/standards/sfs
 */
inline bool is_ogc_type(std::string_view type_lcase)
{
    static const std::string_view Prefix = "st_";
    if (type_lcase.find(Prefix) == 0)
        type_lcase.remove_prefix(Prefix.size());
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
        equals(type_lcase));
}

inline column_type iso_type(std::string_view type_lcase, int scale)
{
    if ((within(type_lcase)("int") && !within(type_lcase)("interval")) ||
        type_lcase.find("bool") == 0)
        return column_type::Integer;
    if (any_of({"date", "time", "char", "clob", "text"}, within(type_lcase)))
        return column_type::Text;
    if (any_of({"real", "float", "double"}, within(type_lcase)))
        return column_type::Real;
    if (any_of({"decimal", "number", "numeric"}, equals(type_lcase)))
        return scale ? column_type::Real : column_type::Integer;
    if (any_of({"binary", "blob"}, within(type_lcase)))
        return column_type::Blob;
    return column_type::Invalid;
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

inline void ogc_window_clause(sql_builder& bld,
                              std::string_view col_nm,
                              const geometry::box& extent)
{
    using namespace geometry;
    bld << "ST_EnvIntersects(" << id(col_nm) << ", " << param{left(extent)}
        << ", " << param{bottom(extent)} << ", " << param{right(extent)} << ", "
        << param{top(extent)} << ")";
}

inline auto ogc_decoder()
{
    return [](sql_builder& bld, std::string_view col_nm) {
        bld << "ST_AsBinary(" << id(col_nm) << ") AS " << id(col_nm);
    };
}

inline auto ogc_encoder(int srid)
{
    return [srid](sql_builder& bld, variant_t val) {
        bld << "ST_GeomFromWKB(" << param{val} << ", " << srid << ")";
    };
}

template <class ColumnNames>
auto index_name(const qualified_name& tbl, const ColumnNames& cols)
{
    std::ostringstream os;
    os << "idx_" << tbl.back() << "_" << list{cols, "_"};
    return id(os.str());
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
