// Andrew Naplavkov

#ifndef BARK_DB_TABLE_GUIDE_HPP
#define BARK_DB_TABLE_GUIDE_HPP

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>

namespace bark::db {

inline bool test(const variant_t& var)
{
    return !is_null(var) && boost::lexical_cast<int64_t>(var);
}

template <class T>
class table_guide {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    meta::table load_table(const qualified_name& tbl_nm)
    {
        meta::table res;
        res.name = tbl_nm;
        load_columns(res);
        load_indexes(res);
        return res;
    }

private:
    meta::column make_column(const qualified_name& tbl_nm,
                             std::string_view name,
                             std::string_view type,
                             int scale)
    {
        meta::column res;
        auto& dial = as_mixin().as_dialect();
        res.name = name;
        res.type = dial.type(type, scale);
        if (meta::column_type::Geometry == res.type)
            as_mixin().prepare_geometry_column(tbl_nm, res, type);
        else if (meta::column_type::Text == res.type)
            res.decoder = [type = dial.type_name(res.type)](
                              sql_builder& bld, std::string_view col_nm) {
                bld << "CAST(" << id(col_nm) << " AS " << type << ") AS "
                    << id(col_nm);
            };
        return res;
    }

    void load_columns(meta::table& tbl)
    {
        enum columns { Name, Type, Scale };
        auto bld = builder(as_mixin());
        as_mixin().as_dialect().columns_sql(bld, tbl.name);
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : select(rows)) {
            auto name = boost::lexical_cast<std::string>(row[Name]);
            auto type = boost::lexical_cast<std::string>(row[Type]);
            auto scale =
                is_null(row[Scale]) ? -1 : boost::lexical_cast<int>(row[Scale]);
            auto col = make_column(tbl.name, name, type, scale);
            if (col.type == meta::column_type::Invalid)
                std::cerr << "unknown type: " << type << "("
                          << id(tbl.name, col.name) << ")" << std::endl;
            else
                tbl.columns.push_back(col);
        }
        if (tbl.columns.empty())
            throw std::runtime_error(concat("no columns: ", tbl.name));
    }

    void load_indexes(meta::table& tbl)
    {
        enum columns { Schema, Name, Column, Primary, Descending };
        auto idx_nm = qualified_name{};
        auto idx = meta::index{};
        auto bld = builder(as_mixin());
        as_mixin().as_dialect().indexes_sql(bld, tbl.name);
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : select(rows)) {
            auto name = id(boost::lexical_cast<std::string>(row[Schema]),
                           boost::lexical_cast<std::string>(row[Name]));
            if (name != idx_nm) {
                if (meta::index_type::Invalid != idx.type)
                    tbl.indexes.push_back(idx);

                idx_nm = name;
                idx = meta::index();
                idx.type = test(row[Primary]) ? meta::index_type::Primary
                                              : meta::index_type::Secondary;
            }

            auto col = boost::lexical_cast<std::string>(row[Column]);
            idx.columns.push_back(col);
            if (db::find(tbl.columns, col) == tbl.columns.end() ||
                test(row[Descending]))
                idx.type =
                    meta::index_type::Invalid;  // expression or descending
        }
        if (meta::index_type::Invalid != idx.type)
            tbl.indexes.push_back(idx);
    }
};

}  // namespace bark::db

#endif  // BARK_DB_TABLE_GUIDE_HPP
