// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_DETAIL_TABLE_GUIDE_HPP
#define BARK_DB_SQLITE_DETAIL_TABLE_GUIDE_HPP

#include <bark/unicode.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>

namespace bark {
namespace db {
namespace sqlite {
namespace detail {

template <typename T>
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
        auto& dlct = as_mixin().as_dialect();
        auto bld = builder(as_mixin());
        bld << "PRAGMA TABLE_INFO(" << tbl.name << ")";
        for (auto& row : fetch_all(as_mixin(), bld)) {
            column_def col;
            col.name = boost::lexical_cast<std::string>(row[1]);
            auto type_lcase =
                unicode::to_lower(boost::lexical_cast<std::string>(row[2]));
            col.type = dlct.type(type_lcase, -1);
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
        index_def idx;
        idx.type = index_type::Primary;
        auto bld = builder(as_mixin());
        bld << "PRAGMA TABLE_INFO(" << tbl.name << ")";
        for (auto& row : fetch_all(as_mixin(), bld)) {
            if (dataset::is_null(row[5]))
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
        auto bld = builder(as_mixin());
        bld << "PRAGMA INDEX_INFO(" << idx_nm << ")";
        index_def idx;
        idx.type = index_type::Secondary;
        for (auto& row : fetch_all(as_mixin(), bld))
            resize_and_assign(idx.columns,
                              boost::lexical_cast<int>(row[0]),
                              boost::lexical_cast<std::string>(row[2]));
        return idx;
    }

    void load_indexes(table_def& tbl)
    {
        auto bld = builder(as_mixin());
        bld << "PRAGMA INDEX_LIST(" << tbl.name << ")";
        for (auto& row : fetch_all(as_mixin(), bld)) {
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
            << param(tbl.name.back()) << ")";
        for (auto& row : fetch_all(as_mixin(), bld)) {
            index_def idx;
            idx.type = index_type::Secondary;
            idx.columns.push_back(boost::lexical_cast<std::string>(row[0]));
            tbl.indexes.push_back(std::move(idx));
        }
    }
};

}  // namespace detail
}  // namespace sqlite
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SQLITE_DETAIL_TABLE_GUIDE_HPP
