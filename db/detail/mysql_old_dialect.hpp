// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_OLD_DIALECT_HPP
#define BARK_DB_MYSQL_OLD_DIALECT_HPP

#include <bark/db/detail/mysql_dialect.hpp>

namespace bark::db {

struct mysql_old_dialect : mysql_dialect {
    void projections_sql(sql_builder& bld) override
    {
        bld << "SELECT 0, 0, NULL LIMIT 0";
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

    void projection_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        bld << "(SELECT ST_SRID(" << id(col) << ") FROM " << qualifier(col_nm)
            << " LIMIT 1) UNION ALL (SELECT column_comment FROM "
               "information_schema.columns WHERE LOWER(table_schema) = LOWER("
            << param{scm} << ") AND LOWER(table_name) = LOWER(" << param{tbl}
            << ") AND LOWER(column_name) = LOWER(" << param{col} << "))";
    }

    sql_decoder geom_decoder() override { return st_as_binary(); }

    sql_encoder geom_encoder(std::string_view, int srid) override
    {
        return st_geom_from_wkb(srid);
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
FROM (SELECT ST_ExteriorRing(ST_Envelope()"
            << id(col_nm.back()) << ")) e FROM " << qualifier(col_nm) << ") t";
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const qualified_name& col_nm,
                                 int srid) override
    {
        bld << "ALTER TABLE " << qualifier(col_nm) << " ADD "
            << id(col_nm.back()) << " GEOMETRY NOT NULL COMMENT "
            << param{std::to_string(srid)};
    }
};

}  // namespace bark::db

#endif  // BARK_DB_MYSQL_OLD_DIALECT_HPP
