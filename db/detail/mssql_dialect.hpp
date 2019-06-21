// Andrew Naplavkov

#ifndef BARK_DB_MSSQL_DIALECT_HPP
#define BARK_DB_MSSQL_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/sql_builder_ops.hpp>
#include <bark/db/detail/table_def_ops.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>

namespace bark::db {

class mssql_dialect : public dialect {
public:
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

    column_type type(std::string_view type_lcase, int scale) override
    {
        if (any_of({"geometry", "geography"}, equals(type_lcase)))
            return column_type::Geometry;
        if (type_lcase == "bit")
            return column_type::Integer;
        if (type_lcase == "image")
            return column_type::Blob;
        return iso_type(type_lcase, scale);
    }

    void projection_sql(sql_builder& bld,
                        const qualified_name& col_nm,
                        std::string_view) override
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
            << param{tbl_nm} << ") ORDER BY name, key_ordinal";
    }

    column_decoder geometry_decoder() override
    {
        return [](sql_builder& bld, std::string_view col_nm) {
            bld << id(col_nm) << ".STAsBinary() AS " << id(col_nm);
        };
    }

    column_encoder geometry_encoder(std::string_view type_lcase,
                                    int srid) override
    {
        return [type = std::string{type_lcase}, srid](sql_builder& bld,
                                                      variant_t val) {
            bld << type << "::STGeomFromWKB(" << param{val} << ", " << srid
                << ")";
        };
    }

    void extent_sql(sql_builder& bld,
                    const qualified_name& col_nm,
                    std::string_view type_lcase) override
    {
        bld << "SELECT COUNT(1), " << type_lcase << "::EnvelopeAggregate("
            << id(col_nm.back()) << ").STAsBinary() FROM " << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const table_def& tbl,
                       std::string_view col_nm,
                       const geometry::box& extent) override
    {
        auto blob = geometry::as_binary(extent);
        bld << id(col_nm) << ".Filter("
            << encoder{*db::find(tbl.columns, col_nm), blob} << ") = 1";
    }

    void current_schema_sql(sql_builder& bld) override
    {
        bld << "SELECT SCHEMA_NAME()";
    }

    std::string type_name(column_type type) override
    {
        switch (type) {
            case column_type::Blob:
                return "varbinary(max)";
            case column_type::Integer:
                return "bigint";
            case column_type::Real:
                return "float";
            default:
                return "nvarchar(250)";
        }
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const table_def& tbl,
                                 std::string_view col_nm,
                                 int srid) override
    {
        auto& scm = tbl.name.at(-2);
        bld << "ALTER TABLE " << tbl.name << " ADD " << id(col_nm)
            << " geometry;\nEXEC sp_addextendedproperty @name = "
            << param{"SRID"} << ", @value = " << srid
            << ",\n\t@level0type = " << param{"Schema"}
            << ", @level0name = " << param{scm}
            << ",\n\t@level1type = " << param{"Table"}
            << ", @level1name = " << param{tbl.name.back()}
            << ",\n\t@level2type = " << param{"Column"}
            << ", @level2name = " << param{col_nm};
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const table_def& tbl,
                                  const index_def& idx) override
    {
        using namespace geometry;
        if (!boost::algorithm::any_of(tbl.indexes, same{index_type::Primary}))
            return;
        auto col_nm = idx.columns.front();
        auto ext = extent(*db::find(tbl.columns, col_nm));
        bld << "CREATE SPATIAL INDEX " << index_name(tbl.name, idx.columns)
            << " ON " << tbl.name << " (" << id(col_nm)
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
