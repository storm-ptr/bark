// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_DB2_DIALECT_HPP
#define BARK_DB_DETAIL_DB2_DIALECT_HPP

#include <bark/db/detail/common.hpp>
#include <bark/db/detail/dialect.hpp>
#include <bark/db/sql_builder_ops.hpp>
#include <bark/db/table_def_ops.hpp>
#include <bark/geometry/as_binary.hpp>

namespace bark {
namespace db {
namespace detail {

class db2_dialect : public dialect {
public:
    void projections_sql(sql_builder& bld) override
    {
        bld << "SELECT srs_id, organization_coordsys_id, NULL FROM "
               "db2gse.st_spatial_reference_systems WHERE LOWER(organization) "
               "= "
            << param("epsg");
    }

    void geometries_sql(sql_builder& bld) override
    {
        bld << "SELECT table_schema, table_name, column_name FROM "
               "db2gse.st_geometry_columns";
    }

    void columns_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& scm = reverse_at(tbl_nm, 1);
        bld << "SELECT colname, (CASE typeschema WHEN " << param("SYSIBM")
            << " THEN typename WHEN " << param("DB2GSE")
            << " THEN typename ELSE RTRIM(typeschema) || " << param(".")
            << " || typename END), scale FROM syscat.columns WHERE tabschema = "
            << param(scm) << " AND tabname = " << param(tbl_nm.back())
            << " ORDER BY colno";
    }

    column_type type(boost::string_view type_lcase, int scale) override
    {
        if (is_ogc_type(type_lcase))
            return column_type::Geometry;
        if (any_of({"graphic", "vargraphic"}, equal_to(type_lcase)))
            return column_type::Text;
        return iso_type(type_lcase, scale);
    }

    void projection_sql(sql_builder& bld,
                        const qualified_name& col_nm,
                        boost::string_view) override
    {
        auto& col = col_nm.back();
        auto& tbl = reverse_at(col_nm, 1);
        auto& scm = reverse_at(col_nm, 2);
        bld << "SELECT srs_id FROM db2gse.st_geometry_columns WHERE "
               "table_schema = "
            << param(scm) << " AND table_name = " << param(tbl)
            << " AND column_name = " << param(col);
    }

    void indexes_sql(sql_builder& bld, const qualified_name& tbl_nm) override
    {
        auto& scm = reverse_at(tbl_nm, 1);
        bld << "SELECT RTRIM(i.indschema), i.indname, colname, uniquerule = "
            << param("P") << ", colorder = " << param("D")
            << " FROM syscat.indexes i JOIN syscat.indexcoluse "
               "USING(indschema, indname) WHERE tabschema = "
            << param(scm) << " AND tabname = " << param(tbl_nm.back())
            << " ORDER BY 1, 2, colseq";
    }

    column_decoder geometry_decoder() override
    {
        return [](sql_builder& bld, boost::string_view col_nm) {
            bld << "db2gse.ST_AsBinary(" << id(col_nm) << ") AS " << id(col_nm);
        };
    }

    column_encoder geometry_encoder(boost::string_view type_lcase,
                                    int srid) override
    {
        return [type = type_lcase.to_string(), srid](sql_builder& bld,
                                                     dataset::variant_view v) {
            bld << "db2gse." << type << "(CAST(" << param(v)
                << " AS blob(100M)), " << srid << ")";
        };
    }

    void extent_sql(sql_builder& bld,
                    const qualified_name& col_nm,
                    boost::string_view) override
    {
        auto col = id(col_nm.back());
        bld << "SELECT COUNT(1), MIN(db2gse.ST_MinX(" << col
            << ")), MIN(db2gse.ST_MinY(" << col << ")), MAX(db2gse.ST_MaxX("
            << col << ")), MAX(db2gse.ST_MaxY(" << col << ")) FROM "
            << qualifier(col_nm);
    }

    void window_clause(sql_builder& bld,
                       const table_def& tbl,
                       boost::string_view col_nm,
                       const geometry::box& extent) override
    {
        auto blob = geometry::as_binary(extent);
        bld << "db2gse.EnvelopesIntersect(" << id(col_nm) << ", "
            << encode(column(tbl.columns, col_nm), blob) << ") = 1";
    }

    void current_schema_sql(sql_builder& bld) override
    {
        bld << "VALUES RTRIM(current_schema)";
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
                return "vargraphic(250)";
        }
    }

    void add_geometry_column_sql(sql_builder& bld,
                                 const table_def& tbl,
                                 boost::string_view col_nm,
                                 int srid) override
    {
        bld << "ALTER TABLE " << tbl.name << " ADD " << id(col_nm)
            << " db2gse.st_geometry;\nCALL "
               "db2gse.st_register_spatial_column(\n\tNULL,\n\t"
            << param(id(tbl.name.back())) << ",\n\t" << param(id(col_nm))
            << ",\n\t(SELECT srs_name FROM db2gse.st_spatial_reference_systems "
               "WHERE srs_id = "
            << srid << "),\n\t?,\n\t?)";
    }

    void create_spatial_index_sql(sql_builder& bld,
                                  const table_def& tbl,
                                  const index_def& idx) override
    {
        bld << "CREATE INDEX " << index_name(tbl.name, idx.columns) << " ON "
            << tbl.name << " (" << id(idx.columns.front())
            << ")\n\tEXTEND USING db2gse.spatial_index(1, 0, 0)";
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        iso_page_clause(bld, offset, limit);
    }
};

}  // namespace detail
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_DETAIL_DB2_DIALECT_HPP
