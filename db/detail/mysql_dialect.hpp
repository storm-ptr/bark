// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_DIALECT_HPP
#define BARK_DB_MYSQL_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/db/sql_builder_ops.hpp>
#include <bark/db/table_def_ops.hpp>
#include <bark/geometry/as_binary.hpp>

namespace bark::db {

class mysql_dialect : public dialect {
public:
    void projections_sql(sql_builder& bld) override
    {
        bld << "SELECT 0 AS srid, 0 AS epsg, NULL AS proj4 LIMIT 0";
    }

    void geometries_sql(sql_builder& bld) override
    {
        iso_layers_sql(bld,
                       {"geometry",
                        "point",
                        "linestring",
                        "polygon",
                        "geometrycollection",
                        "multipoint",
                        "multilinestring",
                        "multipolygon"});
    }

    void columns_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        iso_columns_sql(bld, tbl_nm);
    }

    column_type type(std::string_view type_lcase, int scale) override
    {
        if (is_ogc_type(type_lcase))
            return column_type::Geometry;
        if (any_of({"enum", "json"}, equal_to(type_lcase)))
            return column_type::Text;
        if (type_lcase == "fixed")
            return scale ? column_type::Real : column_type::Integer;
        return iso_type(type_lcase, scale);
    }

    void projection_sql(sql_builder& bld,
                        const qualified_name& col_nm,
                        std::string_view) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        bld << "(SELECT ST_SRID(" << id(col) << ") FROM " << qualifier(col_nm)
            << " LIMIT 1) UNION ALL (SELECT COLUMN_COMMENT FROM "
               "INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = "
            << param{scm} << " AND TABLE_NAME = " << param{tbl}
            << " AND COLUMN_NAME = " << param{col} << ")";
    }

    void indexes_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& scm = tbl_nm.at(-2);
        bld << "SELECT NULL, index_name, column_name, index_name = "
            << param{"PRIMARY"} << ", collation = " << param{"D"}
            << " FROM information_schema.statistics WHERE table_schema = "
            << param{scm} << " AND table_name = " << param{tbl_nm.back()}
            << " ORDER BY index_name, seq_in_index";
    }

    column_decoder geometry_decoder() override { return ogc_decoder(); }

    column_encoder geometry_encoder(std::string_view, int srid) override
    {
        return ogc_encoder(srid);
    }

    void extent_sql(sql_builder& bld,
                    const qualified_name& col_nm,
                    std::string_view) override
    {
        ogc_extent_sql(bld, col_nm);
    }

    void window_clause(sql_builder& bld,
                       const table_def& tbl,
                       std::string_view col_nm,
                       const geometry::box& extent) override
    {
        auto blob = geometry::as_binary(extent);
        bld << "MBRIntersects(" << id(col_nm) << ", "
            << encoder{*db::find(tbl.columns, col_nm), blob} << ") = 1";
    }

    void current_schema_sql(sql_builder& bld) override
    {
        bld << "SELECT schema()";
    }

    std::string type_name(column_type type) override
    {
        switch (type) {
            case column_type::Blob:
                return "blob";
            case column_type::Integer:
                return "bigint";
            case column_type::Real:
                return "double";
            default:
                return "char(250) character set utf8";
        }
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const table_def& tbl,
                                 std::string_view col_nm,
                                 int srid) override
    {
        bld << "ALTER TABLE " << tbl.name << " ADD " << id(col_nm)
            << " GEOMETRY NOT NULL COMMENT " << param{std::to_string(srid)};
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const table_def& tbl,
                                  const index_def& idx) override
    {
        bld << "CREATE SPATIAL INDEX " << index_name(tbl.name, idx.columns)
            << " ON " << tbl.name << " (" << id(idx.columns.front()) << ")";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        limit_page_clause(bld, offset, limit);
    }
};

}  // namespace bark::db

#endif  // BARK_DB_MYSQL_DIALECT_HPP
