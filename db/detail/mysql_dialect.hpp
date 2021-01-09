// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_DIALECT_HPP
#define BARK_DB_MYSQL_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/meta_ops.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/as_binary.hpp>

namespace bark::db {

struct mysql_dialect : dialect {
    void projections_sql(sql_builder& bld) override
    {
        bld << "SELECT srs_id, organization_coordsys_id, NULL FROM "
               "information_schema.st_spatial_reference_systems WHERE "
               "LOWER(organization) = "
            << param{"epsg"};
    }

    void geometries_sql(sql_builder& bld) override
    {
        bld << "SELECT table_schema, table_name, column_name FROM "
               "information_schema.st_geometry_columns";
    }

    void columns_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        iso_columns_sql(bld, tbl_nm);
    }

    meta::column_type type(std::string_view type, int scale) override
    {
        if (is_ogc_type(type))
            return meta::column_type::Geometry;
        if (any_of({"enum", "json"}, equals(type)))
            return meta::column_type::Text;
        if (type == "fixed")
            return scale ? meta::column_type::Real : meta::column_type::Integer;
        return iso_type(type, scale);
    }

    void projection_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        bld << "SELECT srs_id FROM information_schema.st_geometry_columns "
               "WHERE LOWER(table_schema) = LOWER("
            << param{scm} << ") AND LOWER(table_name) = LOWER(" << param{tbl}
            << ") AND LOWER(column_name) = LOWER(" << param{col} << ")";
    }

    void indexes_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& tbl = tbl_nm.back();
        auto& scm = tbl_nm.at(-2);
        bld << "SELECT NULL, index_name, column_name, index_name = "
            << param{"PRIMARY"} << " AS is_primary, collation = " << param{"D"}
            << " FROM information_schema.statistics WHERE LOWER(table_schema) "
               "= LOWER("
            << param{scm} << ") AND LOWER(table_name) = LOWER(" << param{tbl}
            << ") ORDER BY is_primary DESC, index_name, seq_in_index";
    }

    sql_decoder geom_decoder() override
    {
        return [](sql_builder& bld, std::string_view col_nm) {
            bld << "ST_AsBinary(" << id(col_nm) << ", "
                << param{"axis-order=long-lat"} << ") AS " << id(col_nm);
        };
    }

    sql_encoder geom_encoder(std::string_view, int srid) override
    {
        return [srid](sql_builder& bld, variant_t val) {
            bld << "ST_GeomFromWKB(" << param{val} << ", " << srid << ", "
                << param{"axis-order=long-lat"} << ")";
        };
    }

    void extent_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        bld << R"(
SELECT
    COUNT(1),
    Min(ST_X(ST_PointN(e, 1))),
    Min(ST_Y(ST_PointN(e, 1))),
    Max(ST_X(ST_PointN(e, 3))),
    Max(ST_Y(ST_PointN(e, 3)))
FROM (SELECT ST_ExteriorRing(ST_Envelope(ST_GeomFromWKB(ST_AsBinary()"
            << id(col_nm.back()) << ", 'axis-order=long-lat')))) e FROM "
            << qualifier(col_nm) << ") t";
    }

    void window_clause(sql_builder& bld,
                       const meta::table& tbl,
                       std::string_view col_nm,
                       const geometry::box& ext) override
    {
        auto blob = geometry::as_binary(ext);
        bld << "MBRIntersects(" << id(col_nm) << ", "
            << encode(*db::find(tbl.columns, col_nm), blob) << ") = 1";
    }

    void current_schema_sql(sql_builder& bld) override
    {
        bld << "SELECT schema()";
    }

    std::string type_name(meta::column_type type) override
    {
        switch (type) {
            case meta::column_type::Blob:
                return "blob";
            case meta::column_type::Integer:
                return "bigint";
            case meta::column_type::Real:
                return "double";
            default:
                return "char(250) character set utf8";
        }
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const qualified_name& col_nm,
                                 int srid) override
    {
        bld << "ALTER TABLE " << qualifier(col_nm) << " ADD "
            << id(col_nm.back()) << " GEOMETRY NOT NULL SRID " << srid;
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const qualified_name& col_nm,
                                  const geometry::box&) override
    {
        bld << "CREATE SPATIAL INDEX " << index_name(col_nm) << " ON "
            << qualifier(col_nm) << " (" << id(col_nm.back()) << ")";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        limit_page_clause(bld, offset, limit);
    }
};

}  // namespace bark::db

#endif  // BARK_DB_MYSQL_DIALECT_HPP
