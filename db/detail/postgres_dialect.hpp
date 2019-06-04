// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_DIALECT_HPP
#define BARK_DB_POSTGRES_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/db/sql_builder_ops.hpp>
#include <bark/db/table_def_ops.hpp>
#include <bark/geometry/as_binary.hpp>

namespace bark::db {

class postgres_dialect : public dialect {
public:
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
        bld << "SELECT column_name, (CASE data_type WHEN "
            << param{"USER-DEFINED"}
            << " THEN udt_name ELSE data_type END), numeric_scale FROM "
               "information_schema.columns WHERE table_schema = "
            << param{scm} << " AND table_name = " << param{tbl_nm.back()}
            << " ORDER BY ordinal_position";
    }

    column_type type(std::string_view type_lcase, int scale) override
    {
        if (within(type_lcase)("array"))
            return column_type::Invalid;
        if (any_of({"geography", "geometry"}, equal_to(type_lcase)))
            return column_type::Geometry;
        if (within(type_lcase)("serial"))
            return column_type::Integer;
        if (any_of({"json", "jsonb", "xml", "hstore"}, equal_to(type_lcase)))
            return column_type::Text;
        if (type_lcase == "bytea")
            return column_type::Blob;
        return iso_type(type_lcase, scale);
    }

    void projection_sql(sql_builder& bld,
                        const qualified_name& col_nm,
                        std::string_view type_lcase) override
    {
        if (type_lcase == "geography")
            bld << "SELECT 4326";
        else
            ogc_projection_sql(bld, col_nm);
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

    column_decoder geometry_decoder() override { return ogc_decoder(); }

    column_encoder geometry_encoder(std::string_view type_lcase,
                                    int srid) override
    {
        if (type_lcase == "geography")
            return [](sql_builder& bld, variant_t v) {
                bld << "ST_GeogFromWKB(" << param{v} << ")";
            };
        else
            return ogc_encoder(srid);
    }

    void extent_sql(sql_builder& bld,
                    const qualified_name& col_nm,
                    std::string_view type_lcase) override
    {
        if (type_lcase == "geography")
            ogc_extent_sql(bld, col_nm);
        else
            bld << "SELECT COUNT(1), ST_AsBinary(ST_Extent("
                << id(col_nm.back()) << ")) FROM " << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const table_def& tbl,
                       std::string_view col_nm,
                       const geometry::box& extent) override
    {
        auto blob = geometry::as_binary(extent);
        bld << id(col_nm) << " && "
            << encoder{*db::find(tbl.columns, col_nm), blob};
    }

    void current_schema_sql(sql_builder& bld) override
    {
        bld << "SELECT current_schema()";
    }

    std::string type_name(column_type type) override
    {
        switch (type) {
            case column_type::Blob:
                return "bytea";
            case column_type::Integer:
                return "bigint";
            case column_type::Real:
                return "float";
            default:
                return "text";
        }
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const table_def& tbl,
                                 std::string_view col_nm,
                                 int srid) override
    {
        auto& scm = tbl.name.at(-2);
        bld << "SELECT AddGeometryColumn(" << param{scm} << ", "
            << param{tbl.name.back()} << ", " << param{col_nm} << ", " << srid
            << ", " << param{"GEOMETRY"} << ", 2)";
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const table_def& tbl,
                                  const index_def& idx) override
    {
        bld << "CREATE INDEX ON " << tbl.name << " USING GIST ("
            << list{idx.columns, ", ", id<>} << ")";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        return iso_page_clause(bld, offset, limit);
    }
};

}  // namespace bark::db

#endif  // BARK_DB_POSTGRES_DIALECT_HPP
