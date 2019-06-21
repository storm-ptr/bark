// Andrew Naplavkov

#ifndef BARK_DB_TABLE_DEF_OPS_HPP
#define BARK_DB_TABLE_DEF_OPS_HPP

#include <bark/db/table_def.hpp>
#include <bark/detail/unicode.hpp>
#include <bark/geometry/envelope.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
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
    return boost::algorithm::any_of(indexes, [&](auto& idx) {
        return boost::range::search(idx.columns,
                                    col_nms,
                                    unicode::case_insensitive_equal_to{}) ==
               idx.columns.begin();
    });
}

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
