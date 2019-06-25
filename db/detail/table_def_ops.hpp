// Andrew Naplavkov

#ifndef BARK_DB_TABLE_DEF_OPS_HPP
#define BARK_DB_TABLE_DEF_OPS_HPP

#include <bark/db/table_def.hpp>
#include <bark/detail/grid.hpp>
#include <bark/detail/unicode.hpp>
#include <bark/geometry/envelope.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/search.hpp>
#include <cmath>

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

template <class Rng>
auto names(Rng&& rng)
{
    return as<std::vector<std::string>>(rng,
                                        [&](auto& item) { return item.name; });
}

inline rtree make_tiles(size_t count, geometry::box ext)
{
    constexpr size_t RowsPerTile = 2000;
    rtree res;
    if (count) {
        auto side = size_t(ceil(sqrt(count / double(RowsPerTile))));
        grid gr(ext, side + 1, side + 1);
        for (size_t row = 1; row < gr.rows(); ++row)
            for (size_t col = 1; col < gr.cols(); ++col)
                res.insert({{gr.x(row - 1, col - 1), gr.y(row - 1, col - 1)},
                            {gr.x(row, col), gr.y(row, col)}});
    }
    return res;
}

inline auto decode(const column_def& col)
{
    return streamable([&](sql_builder& bld) { col.decoder(bld, col.name); });
}

inline auto encode(const column_def& col, const variant_t& var)
{
    return streamable([&](sql_builder& bld) { col.encoder(bld, var); });
}

}  // namespace bark::db

#endif  // BARK_DB_TABLE_DEF_OPS_HPP
