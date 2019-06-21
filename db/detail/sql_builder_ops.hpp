// Andrew Naplavkov

#ifndef BARK_DB_SQL_BUILDER_OPS_HPP
#define BARK_DB_SQL_BUILDER_OPS_HPP

#include <bark/db/table_def.hpp>
#include <boost/range/combine.hpp>

namespace bark::db {

struct decoder {
    const column_def& col;

    friend sql_builder& operator<<(sql_builder& bld, const decoder& that)
    {
        that.col.decoder(bld, that.col.name);
        return bld;
    }
};

template <class T>
struct encoder {
    const column_def& col;
    const T& val;

    friend sql_builder& operator<<(sql_builder& bld, const encoder& that)
    {
        that.col.encoder(bld, that.val);
        return bld;
    }
};

template <class T>
encoder(column_def, T)->encoder<T>;

template <class Columns>
struct row_encoder {
    const Columns& cols;
    const std::vector<variant_t>& row;

    friend sql_builder& operator<<(sql_builder& bld, const row_encoder& that)
    {
        return bld << "("
                   << list{boost::combine(that.cols, that.row),
                           ",",
                           [](const auto& pair) {
                               return encoder{boost::get<0>(pair),
                                              boost::get<1>(pair)};
                           }}
                   << ")";
    }
};

template <class Columns>
row_encoder(Columns, std::vector<variant_t>)->row_encoder<Columns>;

}  // namespace bark::db

#endif  // BARK_DB_SQL_BUILDER_OPS_HPP
