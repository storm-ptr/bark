// Andrew Naplavkov

#ifndef BARK_DB_TABLE_DEF_OPS_HPP
#define BARK_DB_TABLE_DEF_OPS_HPP

#include <algorithm>
#include <bark/db/table_def.hpp>
#include <bark/geometry/as_text.hpp>
#include <bark/geometry/envelope.hpp>
#include <bark/proj/print.hpp>
#include <bark/unicode.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/search.hpp>
#include <stdexcept>

namespace bark::db {

inline geometry::box extent(const column_def& col)
{
    if (col.tiles.empty())
        return {};
    return geometry::envelope(bounds(col.tiles));
}

inline decltype(auto) name(const column_def& col)
{
    return col.name;
}

inline bool is_geom(const column_def& col)
{
    return col.type == column_type::Geometry;
}

inline bool is_not_geom(const column_def& col)
{
    return !is_geom(col);
}

inline bool sortable(const column_def& col)
{
    switch (col.type) {
        case column_type::Integer:
        case column_type::Real:
        case column_type::Text:
            return true;
    }
    return false;
}

template <class Columns>
auto find(Columns&& cols, std::string_view col_nm)
{
    return boost::range::find_if(cols, [&](auto& col) {
        return unicode::case_insensitive_equal_to{}(col_nm, col.name);
    });
}

template <class Columns>
bool contains(Columns&& cols, std::string_view col_nm)
{
    return db::find(cols, col_nm) != std::end(cols);
}

template <class Columns>
decltype(auto) column(Columns&& cols, std::string_view col_nm)
{
    auto it = db::find(cols, col_nm);
    if (it == std::end(cols))
        throw std::out_of_range(std::string{col_nm});
    return *it;
}

template <class Columns, class ColumnNames>
auto columns(Columns&& cols, ColumnNames&& col_nms)
{
    return as<std::vector<column_def>>(
        col_nms, [&](auto& col_nm) { return column(cols, col_nm); });
}

template <class Columns>
auto column_names(Columns&& cols)
{
    return as<std::vector<std::string>>(cols, name);
}

template <class LhsColumns, class RhsColumns>
auto set_difference(LhsColumns&& lhs, RhsColumns&& rhs)
{
    std::vector<column_def> res;
    for (auto& col : lhs)
        if (!contains(rhs, col.name))
            res.push_back(col);
    return res;
}

template <class Indexes, class ColumnNames>
bool indexed(Indexes&& indexes, ColumnNames&& col_nms)
{
    return boost::range::find_if(indexes, [&](auto& idx) {
               return boost::range::search(
                          idx.columns,
                          col_nms,
                          unicode::case_insensitive_equal_to{}) ==
                      idx.columns.begin();
           }) != indexes.end();
}

template <class Indexes>
auto find_primary(Indexes&& indexes)
{
    return boost::range::find_if(
        indexes, [](auto& idx) { return idx.type == index_type::Primary; });
}

inline std::ostream& operator<<(std::ostream& os, const column_def& col)
{
    static const char* Types[] = {
        "INVALID", "BLOB", "GEOMETRY", "INTEGER", "REAL", "TEXT"};
    os << Types[(size_t)col.type] << " " << col.name;
    if (col.type == column_type::Geometry) {
        std::vector<std::string> opts;
        if (!col.projection.empty())
            opts.push_back(proj::print(col.projection));
        if (!col.tiles.empty())
            opts.push_back(geometry::as_text(bounds(col.tiles)));
        if (!opts.empty())
            os << " (" << list{opts, ", "} << ")";
    }
    return os;
}

inline bool operator==(const column_def& lhs, const column_def& rhs)
{
    return unicode::case_insensitive_equal_to{}(lhs.name, rhs.name) &&
           lhs.type == rhs.type &&
           proj::normalize(lhs.projection) == proj::normalize(rhs.projection);
}

inline std::ostream& operator<<(std::ostream& os, const index_def& idx)
{
    static const char* Types[] = {"INVALID", "PRIMARY", "SECONDARY"};
    return os << Types[(size_t)idx.type] << " INDEX ("
              << list{idx.columns, ", "} << ")";
}

inline bool operator==(const index_def& lhs, const index_def& rhs)
{
    return lhs.type == rhs.type &&
           boost::range::equal(
               lhs.columns, rhs.columns, unicode::case_insensitive_equal_to{});
}

inline std::ostream& operator<<(std::ostream& os, const table_def& tbl)
{
    os << "TABLE " << tbl.name << " (\n\t" << list{tbl.columns, ",\n\t"};
    if (!tbl.indexes.empty())
        os << ",\n\t" << list{tbl.indexes, ",\n\t"};
    return os << "\n)";
}

/// the table name is not compared (structure only)
inline bool operator==(const table_def& lhs, const table_def& rhs)
{
    return std::is_permutation(lhs.columns.begin(),
                               lhs.columns.end(),
                               rhs.columns.begin(),
                               rhs.columns.end()) &&
           std::is_permutation(lhs.indexes.begin(),
                               lhs.indexes.end(),
                               rhs.indexes.begin(),
                               rhs.indexes.end());
}

}  // namespace bark::db

#endif  // BARK_DB_TABLE_DEF_OPS_HPP
