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

    column_type type(std::string_view type_lcase, int scale) override
    {
        return is_ogc_type(type_lcase) ? column_type::Geometry
                                       : iso_type(type_lcase, scale);
    }

    void projection_sql(sql_builder& bld,
                        const qualified_name& col_nm,
                        std::string_view) override
    {
        ogc_projection_sql(bld, col_nm);
    }

    void indexes_sql(sql_builder&, const qualified_name&) override
    {
        throw std::logic_error{"not implemented"};
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
        bld << "SELECT COUNT(1), ST_AsBinary(Extent(" << id(col_nm.back())
            << ")) FROM " << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const table_def& tbl,
                       std::string_view col_nm,
                       const geometry::box& extent) override
    {
        using namespace geometry;
        auto cols = {col_nm};
        if (indexed(tbl.indexes, cols))
            bld << "rowid IN (SELECT pkid FROM " << index_name(tbl.name, cols)
                << " WHERE xmax >= " << param{left(extent)}
                << " AND xmin <= " << param{right(extent)}
                << " AND ymax >= " << param{bottom(extent)}
                << " AND ymin <= " << param{top(extent)} << ")";
        else
            ogc_window_clause(bld, col_nm, extent);
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
                                 const table_def& tbl,
                                 std::string_view col_nm,
                                 int srid) override
    {
        bld << "SELECT AddGeometryColumn(" << param{tbl.name.back()} << ", "
            << param{col_nm} << ", " << srid << ", " << param{"GEOMETRY"}
            << ")";
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const table_def& tbl,
                                  const index_def& idx) override
    {
        bld << "SELECT CreateSpatialIndex(" << param{tbl.name.back()} << ", "
            << param{idx.columns.front()} << ")";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        limit_page_clause(bld, offset, limit);
    }
};

}  // namespace bark::db::sqlite

#endif  // BARK_DB_SQLITE_DIALECT_HPP
