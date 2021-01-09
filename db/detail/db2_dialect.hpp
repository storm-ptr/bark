// Andrew Naplavkov

#ifndef BARK_DB_DB2_DIALECT_HPP
#define BARK_DB_DB2_DIALECT_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/meta_ops.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/as_binary.hpp>

namespace bark::db {

struct db2_dialect : dialect {
    void projections_sql(sql_builder& bld) override
    {
        bld << "SELECT srs_id, organization_coordsys_id, NULL FROM "
               "db2gse.st_spatial_reference_systems WHERE LOWER(organization) "
               "= "
            << param{"epsg"};
    }

    void geometries_sql(sql_builder& bld) override
    {
        bld << "SELECT table_schema, table_name, column_name FROM "
               "db2gse.st_geometry_columns";
    }

    void columns_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& tbl = tbl_nm.back();
        auto& scm = tbl_nm.at(-2);
        bld << "SELECT colname, LOWER(CASE typeschema WHEN " << param{"SYSIBM"}
            << " THEN typename WHEN " << param{"DB2GSE"}
            << " THEN typename ELSE RTRIM(typeschema) || " << param{"."}
            << " || typename END), scale FROM syscat.columns WHERE tabschema = "
            << param{scm} << " AND tabname = " << param{tbl}
            << " ORDER BY colno";
    }

    meta::column_type type(std::string_view type, int scale) override
    {
        if (is_ogc_type(type))
            return meta::column_type::Geometry;
        if (any_of({"graphic", "vargraphic"}, equals(type)))
            return meta::column_type::Text;
        return iso_type(type, scale);
    }

    void projection_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        auto& scm = col_nm.at(-3);
        bld << "SELECT srs_id FROM db2gse.st_geometry_columns WHERE "
               "table_schema = "
            << param{scm} << " AND table_name = " << param{tbl}
            << " AND column_name = " << param{col};
    }

    void indexes_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& tbl = tbl_nm.back();
        auto& scm = tbl_nm.at(-2);
        bld << "SELECT indschema, indname, colname, uniquerule = " << param{"P"}
            << " AS is_primary, colorder = " << param{"D"}
            << " FROM syscat.indexes i JOIN syscat.indexcoluse "
               "USING(indschema, indname) WHERE tabschema = "
            << param{scm} << " AND tabname = " << param{tbl}
            << " ORDER BY is_primary DESC, indschema, indname, colseq";
    }

    sql_decoder geom_decoder() override
    {
        return [](sql_builder& bld, std::string_view col_nm) {
            bld << "db2gse.ST_AsBinary(" << id(col_nm) << ") AS " << id(col_nm);
        };
    }

    sql_encoder geom_encoder(std::string_view type, int srid) override
    {
        return [type = std::string{type}, srid](sql_builder& bld, variant_t v) {
            bld << "db2gse." << type << "(CAST(" << param{v}
                << " AS blob(100M)), " << srid << ")";
        };
    }

    void extent_sql(sql_builder& bld, const qualified_name& col_nm) override
    {
        bld << "SELECT COUNT(1), db2gse.ST_AsBinary(db2gse.ST_GetAggrResult("
               "MAX(db2gse.ST_BuildMBRAggr("
            << id(col_nm.back()) << ")))) FROM " << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const meta::table& tbl,
                       std::string_view col_nm,
                       const geometry::box& ext) override
    {
        auto blob = geometry::as_binary(ext);
        bld << "db2gse.EnvelopesIntersect(" << id(col_nm) << ", "
            << encode(*db::find(tbl.columns, col_nm), blob) << ") = 1";
    }

    void current_schema_sql(sql_builder& bld) override
    {
        bld << "VALUES RTRIM(current_schema)";
    }

    std::string type_name(meta::column_type type) override
    {
        switch (type) {
            case meta::column_type::Blob:
                return "blob";
            case meta::column_type::Integer:
                return "bigint";
            case meta::column_type::Real:
                return "double";
            default:
                return "vargraphic(250)";
        }
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const qualified_name& col_nm,
                                 int srid) override
    {
        auto& col = col_nm.back();
        auto& tbl = col_nm.at(-2);
        bld << "ALTER TABLE " << qualifier(col_nm) << " ADD " << id(col)
            << " db2gse.st_geometry;\nCALL "
               "db2gse.st_register_spatial_column(\n\tNULL,\n\t"
            << param{id(tbl)} << ",\n\t" << param{id(col)}
            << ",\n\t(SELECT srs_name FROM db2gse.st_spatial_reference_systems "
               "WHERE srs_id = "
            << srid << "),\n\t?,\n\t?)";
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const qualified_name& col_nm,
                                  const geometry::box&) override
    {
        bld << "CREATE INDEX " << index_name(col_nm) << " ON "
            << qualifier(col_nm) << " (" << id(col_nm.back())
            << ")\n\tEXTEND USING db2gse.spatial_index(1, 0, 0)";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        iso_page_clause(bld, offset, limit);
    }
};

}  // namespace bark::db

#endif  // BARK_DB_DB2_DIALECT_HPP
