// Andrew Naplavkov

#ifndef BARK_DB_SQL_BUILDER_OPS_HPP
#define BARK_DB_SQL_BUILDER_OPS_HPP

#include <bark/dataset/rowset.hpp>
#include <bark/db/table_def.hpp>
#include <boost/range/combine.hpp>

namespace bark {
namespace db {
namespace detail {

struct column_decoder_manipulator {
    const column_def& col;

    friend sql_builder& operator<<(sql_builder& bld,
                                   const column_decoder_manipulator& manip)
    {
        manip.col.decoder(bld, manip.col.name);
        return bld;
    }
};

template <typename T>
struct column_encoder_manipulator {
    const column_def& col;
    const T& val;

    friend sql_builder& operator<<(sql_builder& bld,
                                   const column_encoder_manipulator& manip)
    {
        manip.col.encoder(bld, manip.val);
        return bld;
    }
};

}  // namespace detail

inline auto decode(const column_def& col)
{
    return detail::column_decoder_manipulator{col};
}

template <typename T>
auto encode(const column_def& col, const T& val)
{
    return detail::column_encoder_manipulator<T>{col, val};
}

namespace detail {

struct row_value_manipulator {
    const std::vector<column_def>& cols;
    const dataset::rowset::tuple& row;

    friend sql_builder& operator<<(sql_builder& bld,
                                   const row_value_manipulator& manip)
    {
        return bld << "("
                   << list(boost::combine(manip.cols, manip.row),
                           ",",
                           [](const auto& pair) {
                               return encode(boost::get<0>(pair),
                                             boost::get<1>(pair));
                           })
                   << ")";
    }
};

}  // namespace detail

inline auto row_value_constructor(std::vector<column_def> cols)
{
    return [cols = std::move(cols)](const dataset::rowset::tuple& row) {
        return detail::row_value_manipulator{cols, row};
    };
}

}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SQL_BUILDER_OPS_HPP
