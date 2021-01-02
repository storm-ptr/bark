// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_DIALECT_HPP
#define BARK_DB_POSTGRES_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/meta_ops.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/as_binary.hpp>

namespace bark::db {

struct postgres_dialect : dialect {
    void projections_sql(sql_builder& bld) override
    {
        ogc_projections_sql(bld);
    }

    void geometries_sql(sql_builder& bld) override
    {
        bld << "SELECT f_table_schema, f_table_name, f_geometry_column FROM "
               "geometry_columns UNION ALL SELECT f_table_schema, "
               "f_table_name, f_geography_column FROM geography_columns";
    }

    void columns_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& scm = tbl_nm.at(-2);
        bld << "SELECT column_name, LOWER(CASE data_type WHEN "
            << param{"USER-DEFINED"}
            << " THEN udt_name ELSE data_type END), numeric_scale FROM "
               "information_schema.columns WHERE table_schema = "
            << param{scm} << " AND table_name = " << param{tbl_nm.back()}
            << " ORDER BY ordinal_position";
    }

    meta::column_type type(std::string_view type, int scale) override
    {
        if (within(type)("array"))
            return meta::column_type::Invalid;
        if (any_of({"geography", "geometry"}, equals(type)))
            return meta::column_type::Geometry;
        if (within(type)("serial"))
            return meta::column_type::Integer;
        if (any_of({"json", "jsonb", "xml", "hstore"}, equals(type)))
            return meta::column_type::Text;
        if (type == "bytea")
            return meta::column_type::Blob;
        return iso_type(type, scale);
    }

    void projection_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        bld << "(SELECT srid FROM geometry_columns WHERE "
            << "LOWER(f_table_schema) = LOWER(" << param{scm} << ") AND "
            << "LOWER(f_table_name) = LOWER(" << param{tbl}
            << ") AND LOWER(f_geometry_column) = LOWER(" << param{col}
            << ")) UNION ALL (SELECT srid FROM geography_columns WHERE "
            << "LOWER(f_table_schema) = LOWER(" << param{scm} << ") AND "
            << "LOWER(f_table_name) = LOWER(" << param{tbl}
            << ") AND LOWER(f_geography_column) = LOWER(" << param{col} << "))";
    }

    void indexes_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& scm = tbl_nm.at(-2);
        bld << R"(
WITH indexes AS (
  SELECT s.nspname scm, t.oid tbl, o.relname nm,
         i.indisprimary pri, i.indkey cols, i.indoption opts,
         array_lower(i.indkey, 1) lb, array_upper(i.indkey, 1) ub
  FROM pg_index i, pg_class o, pg_class t, pg_namespace s
  WHERE i.indexrelid = o.oid
    AND i.indrelid = t.oid
    AND t.relnamespace = s.oid
    AND s.nspname = )"
            << param{scm} << R"(
    AND t.relname = )"
            << param{tbl_nm.back()} << R"(
), columns AS (
  SELECT indexes.*, generate_series(lb, ub) col FROM indexes
)
SELECT scm, nm, attname, pri, (opts[col] & 1)
FROM columns, pg_attribute
WHERE attrelid = tbl AND attnum = cols[col])";
    }

    meta::decoder_t geom_decoder() override { return st_as_binary(); }

    meta::encoder_t geom_encoder(std::string_view type, int srid) override
    {
        if (type == "geography")
            return [](sql_builder& bld, variant_t v) {
                bld << "ST_GeogFromWKB(" << param{v} << ")";
            };
        else
            return st_geom_from_wkb(srid);
    }

    void extent_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        bld << "SELECT COUNT(1), ST_AsBinary(ST_Extent(" << id(col_nm.back())
            << "::geometry)) FROM " << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const meta::table& tbl,
                       std::string_view col_nm,
                       const geometry::box& ext) override
    {
        auto blob = geometry::as_binary(ext);
        bld << id(col_nm) << " && "
            << encode(*db::find(tbl.columns, col_nm), blob);
    }

    void current_schema_sql(sql_builder& bld) override
    {
        bld << "SELECT current_schema()";
    }

    std::string type_name(meta::column_type type) override
    {
        switch (type) {
            case meta::column_type::Blob:
                return "bytea";
            case meta::column_type::Integer:
                return "bigint";
            case meta::column_type::Real:
                return "float";
            default:
                return "text";
        }
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const qualified_name& col_nm,
                                 int srid) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        bld << "SELECT AddGeometryColumn(" << param{scm} << ", " << param{tbl}
            << ", " << param{col} << ", " << srid << ", " << param{"GEOMETRY"}
            << ", 2)";
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const qualified_name& col_nm,
                                  const geometry::box&) override
    {
        bld << "CREATE INDEX " << index_name(col_nm) << " ON "
            << qualifier(col_nm) << " USING GIST (" << id(col_nm.back()) << ")";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        iso_page_clause(bld, offset, limit);
    }
};

}  // namespace bark::db

#endif  // BARK_DB_POSTGRES_DIALECT_HPP
