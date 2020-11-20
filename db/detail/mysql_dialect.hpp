// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_DIALECT_HPP
#define BARK_DB_MYSQL_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/table_def_ops.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/as_binary.hpp>

namespace bark::db {

class mysql_dialect : public dialect {
    int version_;

public:
    explicit mysql_dialect(command& cmd)
    {
        exec(cmd, "SELECT version()");
        auto version = fetch_or(cmd, std::string{"5"});
        version_ = std::stoi(version.c_str());
    }

    void projections_sql(sql_builder& bld) override
    {
        if (version_ < 8)
            bld << "SELECT 0 AS srid, 0 AS epsg, NULL AS proj4 LIMIT 0";
        else
            bld << "SELECT srs_id AS srid, organization_coordsys_id AS epsg, "
                   "NULL AS proj4 FROM "
                   "information_schema.st_spatial_reference_systems WHERE "
                   "LOWER(organization) = "
                << param{"epsg"};
    }

    void geometries_sql(sql_builder& bld) override
    {
        if (version_ < 8)
            iso_layers_sql(bld,
                           {"geometry",
                            "point",
                            "linestring",
                            "polygon",
                            "geometrycollection",
                            "multipoint",
                            "multilinestring",
                            "multipolygon"});
        else
            bld << "SELECT table_schema, table_name, column_name FROM "
                   "information_schema.st_geometry_columns";
    }

    void columns_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        iso_columns_sql(bld, tbl_nm);
    }

    column_type type(std::string_view type, int scale) override
    {
        if (is_ogc_type(type))
            return column_type::Geometry;
        if (any_of({"enum", "json"}, equals(type)))
            return column_type::Text;
        if (type == "fixed")
            return scale ? column_type::Real : column_type::Integer;
        return iso_type(type, scale);
    }

    void projection_sql(sql_builder& bld,
                        const qualified_name& col_nm,
                        std::string_view) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        if (version_ < 8)
            bld << "(SELECT ST_SRID(" << id(col) << ") FROM "
                << qualifier(col_nm)
                << " LIMIT 1) UNION ALL (SELECT column_comment FROM "
                   "information_schema.columns WHERE table_schema = "
                << param{scm} << " AND table_name = " << param{tbl}
                << " AND column_name = " << param{col} << ")";
        else
            bld << "SELECT srs_id FROM information_schema.st_geometry_columns "
                   "WHERE LOWER(table_schema) = LOWER("
                << param{scm} << ") AND LOWER(table_name) = LOWER("
                << param{tbl} << ") AND LOWER(column_name) = LOWER("
                << param{col} << ")";
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

    column_decoder geometry_decoder() override
    {
        if (version_ < 8)
            return st_as_binary();
        else
            return [](sql_builder& bld, std::string_view col_nm) {
                bld << "ST_AsBinary(" << id(col_nm) << ", "
                    << param{"axis-order=long-lat"} << ") AS " << id(col_nm);
            };
    }

    column_encoder geometry_encoder(std::string_view, int srid) override
    {
        if (version_ < 8)
            return st_geom_from_wkb(srid);
        else
            return [srid](sql_builder& bld, variant_t val) {
                bld << "ST_GeomFromWKB(" << param{val} << ", " << srid << ", "
                    << param{"axis-order=long-lat"} << ")";
            };
    }

    void extent_sql(sql_builder& bld,
                    const qualified_name& col_nm,
                    std::string_view) override
    {
        if (version_ < 8)
            ogc_extent_sql(bld, col_nm);
        else
            bld << R"(
SELECT
    COUNT(1),
    Min(ST_X(ST_PointN(e, 1))),
    Min(ST_Y(ST_PointN(e, 1))),
    Max(ST_X(ST_PointN(e, 3))),
    Max(ST_Y(ST_PointN(e, 3)))
FROM (
    SELECT ST_ExteriorRing(ST_Envelope(ST_GeomFromWKB(ST_AsBinary()"
                << id(col_nm.back()) << ", 'axis-order=long-lat')))) e FROM "
                << qualifier(col_nm) << ") t";
    }

    void window_clause(sql_builder& bld,
                       const table_def& tbl,
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
            << " GEOMETRY NOT NULL";
        if (version_ < 8)
            bld << " COMMENT " << param{std::to_string(srid)};
        else
            bld << " SRID " << srid;
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
