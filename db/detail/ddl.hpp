// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_DDL_HPP
#define BARK_DB_DETAIL_DDL_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/sql_builder.hpp>
#include <bark/db/table_def_ops.hpp>
#include <bark/proj/epsg.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <sstream>

namespace bark {
namespace db {
namespace detail {

template <typename T>
class ddl {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    table_script make_script(table_def tbl)
    {
        tbl.name = id(as_mixin().cached_schema(), tbl.name.back());
        sql_builder bld{embeded_params(as_mixin().make_command()->syntax())};
        create_table_sql(bld, tbl);
        add_geometry_columns_sql(bld, tbl);
        create_indexes_sql(bld, tbl);
        return {tbl.name, bld.sql()};
    }

private:
    void create_table_sql(sql_builder& bld, const table_def& tbl)
    {
        auto& dlct = as_mixin().as_dialect();
        bld << "CREATE TABLE " << tbl.name << " (\n\t"
            << list(tbl.columns | boost::adaptors::filtered([](auto& col) {
                        return col.type != column_type::Geometry;
                    }),
                    ",\n\t",
                    [&](auto& col) -> column_manipulator {
                        auto cols = {col.name};
                        return {col.name,
                                dlct.type_name(col.type),
                                indexed(tbl.indexes, cols)};
                    });
        auto pri = find_primary(tbl.indexes);
        if (pri != tbl.indexes.end())
            bld << ",\n\tPRIMARY KEY (" << list(pri->columns, ", ", id<>)
                << ")";
        bld << "\n);\n";
    }

    void add_geometry_columns_sql(sql_builder& bld, const table_def& tbl)
    {
        for (auto& col : tbl.columns | boost::adaptors::filtered([](auto& col) {
                             return col.type == column_type::Geometry;
                         })) {
            int srid = 0;
            for (auto pj : {col.projection, proj::epsg().find_proj(4326)}) {
                try {
                    if (!srid)
                        srid = as_mixin().find_srid(pj);
                }
                catch (const std::exception&) {
                }
            }
            as_mixin().as_dialect().add_geometry_column_sql(
                bld, tbl, col.name, srid);
            bld << ";\n";
        }
    }

    void create_indexes_sql(sql_builder& bld, const table_def& tbl)
    {
        for (auto& idx :
             tbl.indexes | boost::adaptors::filtered([&tbl](auto& idx) {
                 return idx.type == index_type::Secondary;
             })) {
            auto& col = column(tbl.columns, idx.columns.front());
            if (col.type == column_type::Geometry)
                as_mixin().as_dialect().create_spatial_index_sql(bld, tbl, idx);
            else
                bld << "CREATE INDEX " << index_name(tbl.name, idx.columns)
                    << " ON " << tbl.name << " ("
                    << list(idx.columns, ", ", id<>) << ")";
            bld << ";\n";
        }
    }

private:
    struct column_manipulator {
        std::string name;
        std::string type;
        bool not_null;

        friend sql_builder& operator<<(sql_builder& bld,
                                       const column_manipulator& manip)
        {
            bld << id(manip.name) << " " << manip.type;
            if (manip.not_null)
                bld << " NOT NULL";
            return bld;
        }
    };
};

}  // namespace detail
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_DETAIL_DDL_HPP
