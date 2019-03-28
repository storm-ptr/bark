// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_TABLE_GUIDE_HPP
#define BARK_DB_SQLITE_TABLE_GUIDE_HPP

#include <bark/detail/unicode.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>

namespace bark::db::sqlite {

template <class T>
class table_guide {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    table_def load_table(const qualified_name& tbl_nm)
    {
        table_def res;
        res.name = tbl_nm;
        load_columns(res);
        load_primary_index(res);
        load_indexes(res);
        load_spatial_indexes(res);
        return res;
    }

private:
    void load_columns(table_def& tbl)
    {
        auto& dial = as_mixin().as_dialect();
        auto bld = builder(as_mixin());
        bld << "PRAGMA TABLE_INFO(" << tbl.name << ")";
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : range(rows)) {
            column_def col;
            col.name = boost::lexical_cast<std::string>(row[1]);
            auto type_lcase =
                unicode::to_lower(boost::lexical_cast<std::string>(row[2]));
            col.type = dial.type(type_lcase, -1);
            if (col.type == column_type::Geometry)
                as_mixin().prepare_geometry_column(tbl.name, col, type_lcase);
            if (col.type == column_type::Invalid)
                std::cerr << "unknown type: " << type_lcase << "("
                          << id(tbl.name, col.name) << ")" << std::endl;
            else
                tbl.columns.push_back(col);
        }
        if (tbl.columns.empty())
            throw std::runtime_error("empty table: " + tbl.name.back());
    }

    void load_primary_index(table_def& tbl)
    {
        auto idx = index_def{};
        idx.type = index_type::Primary;
        auto bld = builder(as_mixin());
        bld << "PRAGMA TABLE_INFO(" << tbl.name << ")";
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : range(rows)) {
            if (is_null(row[5]))
                continue;
            auto key = boost::lexical_cast<int>(row[5]);  // one-based
            if (key == 0)
                continue;
            resize_and_assign(
                idx.columns, key - 1, boost::lexical_cast<std::string>(row[1]));
        }
        if (!idx.columns.empty())
            tbl.indexes.push_back(std::move(idx));
    }

    index_def load_index(const qualified_name& idx_nm)
    {
        auto res = index_def{};
        auto bld = builder(as_mixin());
        bld << "PRAGMA INDEX_INFO(" << idx_nm << ")";
        res.type = index_type::Secondary;
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : range(rows))
            resize_and_assign(res.columns,
                              boost::lexical_cast<int>(row[0]),
                              boost::lexical_cast<std::string>(row[2]));
        return res;
    }

    void load_indexes(table_def& tbl)
    {
        auto bld = builder(as_mixin());
        bld << "PRAGMA INDEX_LIST(" << tbl.name << ")";
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : range(rows)) {
            auto idx = load_index(id(boost::lexical_cast<std::string>(row[1])));
            if (!idx.columns.empty() && !indexed(tbl.indexes, idx.columns))
                tbl.indexes.push_back(std::move(idx));
        }
    }

    void load_spatial_indexes(table_def& tbl)
    {
        auto bld = builder(as_mixin());
        bld << "SELECT f_geometry_column FROM geometry_columns WHERE "
               "spatial_index_enabled AND LOWER(f_table_name) = LOWER("
            << param{tbl.name.back()} << ")";
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : range(rows)) {
            auto idx = index_def{};
            idx.type = index_type::Secondary;
            idx.columns.push_back(boost::lexical_cast<std::string>(row[0]));
            tbl.indexes.push_back(std::move(idx));
        }
    }
};

}  // namespace bark::db::sqlite

#endif  // BARK_DB_SQLITE_TABLE_GUIDE_HPP
