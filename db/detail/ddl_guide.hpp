// Andrew Naplavkov

#ifndef BARK_DB_DDL_GUIDE_HPP
#define BARK_DB_DDL_GUIDE_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/meta_ops.hpp>
#include <bark/db/sql_builder.hpp>
#include <bark/proj/epsg.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <initializer_list>
#include <sstream>

namespace bark::db {

template <class T>
class ddl_guide {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    std::pair<qualified_name, std::string> script(meta::table tbl)
    {
        tbl.name = id(as_mixin().cached_schema(), tbl.name.back());
        sql_builder bld{embeded_params(as_mixin().make_command()->syntax())};
        create_table_sql(bld, tbl);
        add_geometry_columns_sql(bld, tbl);
        create_indexes_sql(bld, tbl);
        return {tbl.name, bld.sql()};
    }

private:
    struct column {
        std::string name;
        std::string type;
        bool not_null;

        friend sql_builder& operator<<(sql_builder& bld, const column& that)
        {
            return bld << id(that.name) << " " << that.type
                       << (that.not_null ? " NOT NULL" : "");
        }
    };

    void create_table_sql(sql_builder& bld, const meta::table& tbl)
    {
        auto& dial = as_mixin().as_dialect();
        bld << "CREATE TABLE " << tbl.name << " (\n\t"
            << list{tbl.columns | boost::adaptors::filtered(std::not_fn(
                                      same{meta::column_type::Geometry})),
                    ",\n\t",
                    [&](auto& col) {
                        auto name = col.name;
                        auto type = dial.type_name(col.type);
                        auto not_null = indexed(
                            tbl.indexes, std::initializer_list{col.name});
                        return column{name, type, not_null};
                    }};
        auto pri =
            boost::range::find_if(tbl.indexes, same{meta::index_type::Primary});
        if (pri != tbl.indexes.end())
            bld << ",\n\tPRIMARY KEY (" << list{pri->columns, ", ", id<>}
                << ")";
        bld << "\n);\n";
    }

    void add_geometry_columns_sql(sql_builder& bld, const meta::table& tbl)
    {
        for (auto&& col : tbl.columns | boost::adaptors::filtered(same{
                                            meta::column_type::Geometry})) {
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
                bld, id(tbl.name, col.name), srid);
            bld << ";\n";
        }
    }

    void create_indexes_sql(sql_builder& bld, const meta::table& tbl)
    {
        for (auto& idx : tbl.indexes | boost::adaptors::filtered(
                                           same{meta::index_type::Secondary})) {
            auto& col = *db::find(tbl.columns, idx.columns.front());
            if (col.type == meta::column_type::Geometry)
                as_mixin().as_dialect().create_spatial_index_sql(
                    bld, id(tbl.name, col.name), geometry::envelope(col.tiles));
            else
                bld << "CREATE INDEX "
                    << index_name(tbl.name.back(), idx.columns) << " ON "
                    << tbl.name << " (" << list{idx.columns, ", ", id<>} << ")";
            bld << ";\n";
        }
    }
};

}  // namespace bark::db

#endif  // BARK_DB_DDL_GUIDE_HPP
