// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_DIALECT_HPP
#define BARK_DB_SQLITE_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/geometry_ops.hpp>

namespace bark::db {

struct sqlite_dialect : dialect {
    void projections_sql(sql_builder& bld) override
    {
        ogc_projections_sql(bld);
    }

    void geometries_sql(sql_builder& bld) override
    {
        bld << "SELECT NULL, t.name, g.f_geometry_column FROM geometry_columns "
               "g JOIN sqlite_master t ON LOWER(g.f_table_name) = "
               "LOWER(t.name)";
    }

    void columns_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& tbl = tbl_nm.back();
        bld << "SELECT name, LOWER(type), NULL FROM pragma_table_info("
            << param{tbl} << ")";
    }

    meta::column_type type(std::string_view type, int scale) override
    {
        return is_ogc_type(type) ? meta::column_type::Geometry
                                 : iso_type(type, scale);
    }

    void projection_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        auto& tbl = col_nm.at(-2);
        auto& col = col_nm.back();
        bld << "SELECT srid FROM geometry_columns WHERE "
            << "LOWER(f_table_name) = LOWER(" << param{tbl}
            << ") AND LOWER(f_geometry_column) = LOWER(" << param{col} << ")";
    }

    void indexes_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& tbl = tbl_nm.back();
        bld << R"(
SELECT
  NULL,
  i.name AS index_name,
  c.name AS column_name,
  i.origin = )"
            << param{"pk"} << R"( AS is_primary,
  c.desc,
  c.seqno AS seqno
FROM pragma_index_list()"
            << param{tbl} << R"() AS i, pragma_index_xinfo(i.name) AS c
WHERE c.key

UNION ALL

SELECT
  NULL,
  )" << param{"PRIMARY"}
            << R"(,
  name,
  1,
  0,
  pk - 1
FROM pragma_table_info()"
            << param{tbl} << R"()
WHERE pk > 0
AND NOT EXISTS(SELECT * FROM pragma_index_list()"
            << param{tbl} << R"() WHERE origin = )" << param{"pk"} << R"()

UNION ALL

SELECT
  NULL,)" << param{"idx_"}
            << R"( || f_table_name || )" << param{"_"}
            << R"( || f_geometry_column,
  f_geometry_column,
  0,
  0,
  0
FROM geometry_columns
WHERE spatial_index_enabled
AND LOWER(f_table_name) = LOWER()"
            << param{tbl} << R"()

ORDER BY is_primary DESC, index_name, seqno
)";
    }

    sql_decoder geom_decoder() override { return st_as_binary(); }

    sql_encoder geom_encoder(std::string_view, int srid) override
    {
        return st_geom_from_wkb(srid);
    }

    void extent_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        bld << "SELECT COUNT(1), ST_AsBinary(Extent(" << id(col_nm.back())
            << ")) FROM " << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const meta::table& tbl,
                       std::string_view col_nm,
                       const geometry::box& ext) override
    {
        using namespace geometry;
        auto cols = {col_nm};
        if (indexed(tbl.indexes, cols))
            bld << "rowid IN (SELECT pkid FROM "
                << index_name(tbl.name.back(), cols)
                << " WHERE xmax >= " << param{left(ext)}
                << " AND xmin <= " << param{right(ext)}
                << " AND ymax >= " << param{bottom(ext)}
                << " AND ymin <= " << param{top(ext)} << ")";
        else
            bld << "ST_EnvIntersects(" << id(col_nm) << ", " << param{left(ext)}
                << ", " << param{bottom(ext)} << ", " << param{right(ext)}
                << ", " << param{top(ext)} << ")";
    }

    void current_schema_sql(sql_builder& bld) override { bld << "SELECT NULL"; }

    std::string type_name(meta::column_type type) override
    {
        switch (type) {
            case meta::column_type::Blob:
                return "blob";
            case meta::column_type::Integer:
                return "integer";
            case meta::column_type::Real:
                return "real";
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
        bld << "SELECT AddGeometryColumn(" << param{tbl} << ", " << param{col}
            << ", " << srid << ", " << param{"GEOMETRY"} << ")";
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const qualified_name& col_nm,
                                  const geometry::box&) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        bld << "SELECT CreateSpatialIndex(" << param{tbl} << ", " << param{col}
            << ")";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        limit_page_clause(bld, offset, limit);
    }
};

}  // namespace bark::db

#endif  // BARK_DB_SQLITE_DIALECT_HPP
