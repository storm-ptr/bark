// Andrew Naplavkov

#ifndef BARK_DB_DIALECT_HPP
#define BARK_DB_DIALECT_HPP

#include <bark/db/meta.hpp>
#include <bark/db/sql_builder.hpp>
#include <bark/geometry/geometry.hpp>
#include <memory>

namespace bark::db {

struct dialect {
    virtual ~dialect() = default;

    /// SCHEMA_NAME
    virtual void current_schema_sql(sql_builder&) = 0;

    /// TABLE_SCHEMA, TABLE_NAME, COLUMN_NAME
    virtual void geometries_sql(sql_builder&) = 0;

    /// SRID, EPSG, PROJ4
    virtual void projections_sql(sql_builder&) = 0;

    /// COLUMN_NAME, DATA_TYPE, NUMERIC_SCALE
    virtual void columns_sql(sql_builder&, const qualified_name& tbl_nm) = 0;

    /// INDEX_SCHEMA, INDEX_NAME, COLUMN_NAME, IS_PRIMARY, IS_DESCENDING
    virtual void indexes_sql(sql_builder&, const qualified_name& tbl_nm) = 0;

    /// SRID
    virtual void projection_sql(sql_builder&, const qualified_name& col_nm) = 0;

    /// COUNT, ((XMIN, YMIN, XMAX, YMAX) | EXTENT)
    virtual void extent_sql(sql_builder&, const qualified_name& col_nm) = 0;

    virtual void add_geometry_column_sql(sql_builder&,
                                         const qualified_name& col_nm,
                                         int srid) = 0;

    virtual void create_spatial_index_sql(sql_builder&,
                                          const qualified_name& col_nm,
                                          const geometry::box&) = 0;

    virtual void window_clause(sql_builder&,
                               const meta::table& tbl,
                               std::string_view col_nm,
                               const geometry::box& extent) = 0;

    virtual void page_clause(sql_builder&, size_t offset, size_t limit) = 0;

    virtual meta::column_type type(std::string_view type, int scale) = 0;

    virtual std::string type_name(meta::column_type) = 0;

    virtual sql_decoder geom_decoder() = 0;

    virtual sql_encoder geom_encoder(std::string_view type, int srid) = 0;
};

using dialect_holder = std::unique_ptr<dialect>;

}  // namespace bark::db

#endif  // BARK_DB_DIALECT_HPP
