// Andrew Naplavkov

#ifndef BARK_DB_META_OPS_HPP
#define BARK_DB_META_OPS_HPP

#include <bark/db/meta.hpp>
#include <bark/detail/grid.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/search.hpp>
#include <cmath>

namespace bark::db {

inline bool sortable(const meta::column& col)
{
    switch (col.type) {
        case meta::column_type::Integer:
        case meta::column_type::Real:
        case meta::column_type::Text:
            return true;
        default:
            return false;
    }
}

inline auto decode(const meta::column& col)
{
    return streamable([&](sql_builder& bld) { col.decoder(bld, col.name); });
}

inline auto encode(const meta::column& col, const variant_t& var)
{
    return streamable([&](sql_builder& bld) { col.encoder(bld, var); });
}

template <class Indexes, class ColumnNames>
bool indexed(Indexes&& indexes, ColumnNames&& col_nms)
{
    return boost::algorithm::any_of(indexes, [&](auto& idx) {
        return boost::range::search(idx.columns, col_nms) ==
               idx.columns.begin();
    });
}

template <class Columns>
auto find(Columns&& cols, std::string_view col_nm)
{
    return boost::range::find_if(cols,
                                 [=](auto& col) { return col_nm == col.name; });
}

template <class Columns>
auto names(Columns&& cols)
{
    return as<std::vector<std::string>>(cols,
                                        [&](auto& col) { return col.name; });
}

inline geometry::box_rtree make_tiles(size_t count, geometry::box ext)
{
    constexpr size_t RowsPerTile = 2000;
    geometry::box_rtree res;
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

}  // namespace bark::db

#endif  // BARK_DB_META_OPS_HPP
