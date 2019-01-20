// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_TABLE_GUIDE_HPP
#define BARK_DB_DETAIL_TABLE_GUIDE_HPP

#include <bark/unicode.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>

namespace bark::db::detail {

template <class T>
class table_guide {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    table_def load_table(const qualified_name& tbl_nm)
    {
        table_def res;
        res.name = tbl_nm;
        load_columns(res);
        load_indexes(res);
        return res;
    }

private:
    column_def make_column(const qualified_name& tbl_nm,
                           std::string_view name,
                           std::string_view type_lcase,
                           int scale)
    {
        column_def res;
        auto& dial = as_mixin().as_dialect();
        res.name = name;
        res.type = dial.type(type_lcase, scale);
        if (column_type::Geometry == res.type)
            as_mixin().prepare_geometry_column(tbl_nm, res, type_lcase);
        else if (column_type::Text == res.type)
            res.decoder = [type = dial.type_name(res.type)](
                              sql_builder& bld, std::string_view col_nm) {
                bld << "CAST(" << id(col_nm) << " AS " << type << ") AS "
                    << id(col_nm);
            };
        return res;
    }

    void load_columns(table_def& tbl)
    {
        enum columns { Name, Type, Scale };
        auto bld = builder(as_mixin());
        as_mixin().as_dialect().columns_sql(bld, tbl.name);
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : range(rows)) {
            auto name = boost::lexical_cast<std::string>(row[Name]);
            auto type_lcase =
                unicode::to_lower(boost::lexical_cast<std::string>(row[Type]));
            auto scale =
                is_null(row[Scale]) ? -1 : boost::lexical_cast<int>(row[Scale]);
            auto col = make_column(tbl.name, name, type_lcase, scale);
            if (col.type == column_type::Invalid)
                std::cerr << "unknown type: " << type_lcase << "("
                          << id(tbl.name, col.name) << ")" << std::endl;
            else
                tbl.columns.push_back(col);
        }
        if (tbl.columns.empty())
            throw std::runtime_error(
                "no columns: " + boost::lexical_cast<std::string>(tbl.name));
    }

    void load_indexes(table_def& tbl)
    {
        enum columns { Schema, Name, Column, Primary, Descending };
        auto idx_nm = qualified_name{};
        auto idx = index_def{};
        auto bld = builder(as_mixin());
        as_mixin().as_dialect().indexes_sql(bld, tbl.name);
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : range(rows)) {
            auto name = id(boost::lexical_cast<std::string>(row[Schema]),
                           boost::lexical_cast<std::string>(row[Name]));
            if (name != idx_nm) {
                if (index_type::Invalid != idx.type)
                    tbl.indexes.push_back(idx);

                idx_nm = name;
                idx = index_def();
                idx.type = test(row[Primary]) ? index_type::Primary
                                              : index_type::Secondary;
            }

            auto col = boost::lexical_cast<std::string>(row[Column]);
            idx.columns.push_back(col);
            if (!contains(tbl.columns, col) || test(row[Descending]))
                idx.type = index_type::Invalid;  // expression or descending
        }
        if (index_type::Invalid != idx.type)
            tbl.indexes.push_back(idx);
    }
};

}  // namespace bark::db::detail

#endif  // BARK_DB_DETAIL_TABLE_GUIDE_HPP
