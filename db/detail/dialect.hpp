// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_DIALECT_HPP
#define BARK_DB_DETAIL_DIALECT_HPP

#include <bark/db/sql_builder.hpp>
#include <bark/db/table_def.hpp>
#include <bark/geometry/geometry.hpp>
#include <memory>

namespace bark::db::detail {

struct dialect {
    virtual ~dialect() = default;

    /// SRID, EPSG, PROJ4
    virtual void projections_sql(sql_builder& bld) = 0;

    /// TABLE_SCHEMA, TABLE_NAME, COLUMN_NAME
    virtual void geometries_sql(sql_builder& bld) = 0;

    /// COLUMN_NAME, DATA_TYPE, NUMERIC_SCALE
    virtual void columns_sql(sql_builder& bld,
                             const qualified_name& tbl_nm) = 0;

    /// SRID
    virtual void projection_sql(sql_builder& bld,
                                const qualified_name& col_nm,
                                std::string_view type_lcase) = 0;

    /// INDEX_SCHEMA, INDEX_NAME, IS_PRIMARY, COLUMN_NAME, IS_DESCENDING
    virtual void indexes_sql(sql_builder& bld,
                             const qualified_name& tbl_nm) = 0;

    /// COUNT, ((XMIN, YMIN, XMAX, YMAX) | EXTENT)
    virtual void extent_sql(sql_builder& bld,
                            const qualified_name& col_nm,
                            std::string_view type_lcase) = 0;

    /// SCHEMA_NAME
    virtual void current_schema_sql(sql_builder& bld) = 0;

    virtual void add_geometry_column_sql(sql_builder& bld,
                                         const table_def& tbl,
                                         std::string_view col_nm,
                                         int srid) = 0;

    virtual void create_spatial_index_sql(sql_builder& bld,
                                          const table_def& tbl,
                                          const index_def& idx) = 0;

    virtual void window_clause(sql_builder& bld,
                               const table_def& tbl,
                               std::string_view col_nm,
                               const geometry::box& extent) = 0;

    virtual void page_clause(sql_builder& bld, size_t offset, size_t limit) = 0;

    virtual column_type type(std::string_view type_lcase, int scale) = 0;

    virtual std::string type_name(column_type) = 0;

    virtual column_decoder geometry_decoder() = 0;

    virtual column_encoder geometry_encoder(std::string_view type_lcase,
                                            int srid) = 0;
};

using dialect_holder = std::unique_ptr<dialect>;

}  // namespace bark::db::detail

#endif  // BARK_DB_DETAIL_DIALECT_HPP
