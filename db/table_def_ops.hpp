// Andrew Naplavkov

#ifndef BARK_DB_TABLE_DEF_OPS_HPP
#define BARK_DB_TABLE_DEF_OPS_HPP

#include <bark/db/table_def.hpp>
#include <bark/detail/unicode.hpp>
#include <bark/geometry/as_text.hpp>
#include <bark/geometry/envelope.hpp>
#include <bark/proj/print.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/search.hpp>

namespace bark::db {

inline geometry::box extent(const column_def& col)
{
    if (col.tiles.empty())
        return {};
    return geometry::envelope(bounds(col.tiles));
}

inline bool sortable(const column_def& col)
{
    switch (col.type) {
        case column_type::Integer:
        case column_type::Real:
        case column_type::Text:
            return true;
        default:
            return false;
    }
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

template <class Type>
struct same {
    Type type;

    template <class T>
    bool operator()(const T& val) const
    {
        return type == val.type;
    }
};

template <class T>
same(T)->same<T>;

template <class Type>
struct not_same {
    Type type;

    template <class T>
    auto operator()(const T& val) const
    {
        return type != val.type;
    }
};

template <class T>
not_same(T)->not_same<T>;

template <class Rng>
auto find(Rng&& rng, std::string_view name)
{
    return boost::range::find_if(rng, [&](auto& item) {
        return unicode::case_insensitive_equal_to{}(name, item.name);
    });
}

template <class Rng, class Names>
auto select(Rng&& rng, Names&& names)
{
    return as<std::vector<range_value_t<Rng>>>(
        names, [&](auto& name) { return *db::find(rng, name); });
}

template <class Rng>
auto names(Rng&& rng)
{
    return as<std::vector<std::string>>(rng,
                                        [&](auto& item) { return item.name; });
}

}  // namespace bark::db

#endif  // BARK_DB_TABLE_DEF_OPS_HPP
