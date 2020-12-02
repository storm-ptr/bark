// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_DIALECT_HPP
#define BARK_DB_SQLITE_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/geometry_ops.hpp>

namespace bark::db::sqlite {

class dialect : public db::dialect {
public:
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

    void columns_sql(sql_builder&, const qualified_name&) override
    {
        throw std::logic_error{"not implemented"};
    }

    column_type type(std::string_view type, int scale) override
    {
        return is_ogc_type(type) ? column_type::Geometry
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

    void indexes_sql(sql_builder&, const qualified_name&) override
    {
        throw std::logic_error{"not implemented"};
    }

    column_decoder geometry_decoder() override { return st_as_binary(); }

    column_encoder geometry_encoder(std::string_view, int srid) override
    {
        return st_geom_from_wkb(srid);
    }

    void extent_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        bld << "SELECT COUNT(1), ST_AsBinary(Extent(" << id(col_nm.back())
            << ")) FROM " << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const table_def& tbl,
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

    std::string type_name(column_type type) override
    {
        switch (type) {
            case column_type::Blob:
                return "blob";
            case column_type::Integer:
                return "integer";
            case column_type::Real:
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

}  // namespace bark::db::sqlite

#endif  // BARK_DB_SQLITE_DIALECT_HPP
