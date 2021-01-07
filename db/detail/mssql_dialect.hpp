// Andrew Naplavkov

#ifndef BARK_DB_MSSQL_DIALECT_HPP
#define BARK_DB_MSSQL_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/meta_ops.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/geometry_ops.hpp>

namespace bark::db {

struct mssql_dialect : dialect {
    void projections_sql(sql_builder& bld) override
    {
        bld << "SELECT spatial_reference_id AS srid, "
               "authorized_spatial_reference_id AS epsg, NULL AS proj4 FROM "
               "sys.spatial_reference_systems WHERE LOWER(authority_name) = "
            << param{"epsg"};
    }

    void geometries_sql(sql_builder& bld) override
    {
        iso_layers_sql(bld, {"geometry", "geography"});
    }

    void columns_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        iso_columns_sql(bld, tbl_nm);
    }

    meta::column_type type(std::string_view type, int scale) override
    {
        if (any_of({"geometry", "geography"}, equals(type)))
            return meta::column_type::Geometry;
        if (type == "bit")
            return meta::column_type::Integer;
        if (type == "image")
            return meta::column_type::Blob;
        return iso_type(type, scale);
    }

    void projection_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        bld << "(SELECT TOP 1 " << id(col) << ".STSrid FROM "
            << qualifier(col_nm)
            << ") UNION ALL (SELECT CAST(value AS int) FROM "
               "sys.fn_listextendedproperty("
            << param{"SRID"} << ", " << param{"Schema"} << ", " << param{scm}
            << ", " << param{"Table"} << ", " << param{tbl} << ", "
            << param{"Column"} << ", " << param{col} << "))";
    }

    void indexes_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        bld << "SELECT NULL, name, COL_NAME(c.object_id, column_id), "
               "is_primary_key, is_descending_key FROM sys.indexes i JOIN "
               "sys.index_columns c ON i.object_id = c.object_id AND "
               "i.index_id = c.index_id WHERE i.object_id = OBJECT_ID("
            << param{tbl_nm}
            << ") ORDER BY is_primary_key DESC, name, key_ordinal";
    }

    meta::decoder_t geom_decoder() override
    {
        return [](sql_builder& bld, std::string_view col_nm) {
            bld << id(col_nm) << ".STAsBinary() AS " << id(col_nm);
        };
    }

    meta::encoder_t geom_encoder(std::string_view type, int srid) override
    {
        return [type = std::string{type}, srid](sql_builder& bld, variant_t v) {
            bld << type << "::STGeomFromWKB(" << param{v} << ", " << srid
                << ")";
        };
    }

    void extent_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        // @see
        // https://stackoverflow.com/questions/47725899/sql-geographyenvelopeaggregate-output-not-accurate
        auto col = id(col_nm.back());
        bld << "SELECT COUNT(1), "
               "geometry::EnvelopeAggregate(geometry::STGeomFromWKB("
            << col << ".STAsBinary(), " << col << ".STSrid)).STAsBinary() FROM "
            << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const meta::table& tbl,
                       std::string_view col_nm,
                       const geometry::box& extent) override
    {
        auto blob = geometry::as_binary(extent);
        bld << id(col_nm) << ".Filter("
            << encode(*db::find(tbl.columns, col_nm), blob) << ") = 1";
    }

    void current_schema_sql(sql_builder& bld) override
    {
        bld << "SELECT SCHEMA_NAME()";
    }

    std::string type_name(meta::column_type type) override
    {
        switch (type) {
            case meta::column_type::Blob:
                return "varbinary(max)";
            case meta::column_type::Integer:
                return "bigint";
            case meta::column_type::Real:
                return "float";
            default:
                return "nvarchar(250)";
        }
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const qualified_name& col_nm,
                                 int srid) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        bld << "ALTER TABLE " << qualifier(col_nm) << " ADD " << id(col)
            << " geometry;\nEXEC sp_addextendedproperty @name = "
            << param{"SRID"} << ", @value = " << srid
            << ",\n\t@level0type = " << param{"Schema"}
            << ", @level0name = " << param{scm}
            << ",\n\t@level1type = " << param{"Table"}
            << ", @level1name = " << param{tbl}
            << ",\n\t@level2type = " << param{"Column"}
            << ", @level2name = " << param{col};
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const qualified_name& col_nm,
                                  const geometry::box& ext) override
    {
        using namespace geometry;
        bld << "CREATE SPATIAL INDEX " << index_name(col_nm) << " ON "
            << qualifier(col_nm) << " (" << id(col_nm.back())
            << ")\n\tWITH(BOUNDING_BOX=(" << left(ext) << "," << bottom(ext)
            << "," << right(ext) << "," << top(ext) << "))";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        iso_page_clause(bld, offset, limit);
    }
};

}  // namespace bark::db

#endif  // BARK_DB_MSSQL_DIALECT_HPP
