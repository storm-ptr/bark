// Andrew Naplavkov

#ifndef BARK_DATASET_ROWSET_OPS_HPP
#define BARK_DATASET_ROWSET_OPS_HPP

#include <algorithm>
#include <bark/dataset/rowset.hpp>
#include <bark/unicode.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/combine.hpp>
#include <boost/variant/static_visitor.hpp>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace bark {
namespace dataset {
namespace detail {

class align_visitor : public boost::static_visitor<void> {
    std::ostream& os_;
    size_t width_;

public:
    align_visitor(std::ostream& os, size_t width) : os_(os), width_(width) {}

    void operator()(boost::blank) const { os_ << std::setw(width_) << ""; }

    template <typename T>
    void operator()(T v) const
    {
        os_ << std::setw(width_) << std::right << v;
    }

    void operator()(boost::string_view v) const
    {
        os_ << std::setw(width_) << std::left << v;
    }
};

struct align_manipulator {
    size_t width;
    variant_view view;

    friend std::ostream& operator<<(std::ostream& os,
                                    const align_manipulator& manip)
    {
        os << std::setw(1) << "";
        boost::apply_visitor(align_visitor{os, manip.width}, manip.view);
        return os << std::setw(1) << "";
    }
};

template <typename Row>
struct row_value_manipulator {
    const std::vector<size_t>& widths;
    const Row& row;

    friend std::ostream& operator<<(std::ostream& os,
                                    const row_value_manipulator& manip)
    {
        return os << list(boost::combine(manip.widths, manip.row),
                          "|",
                          [](const auto& pair) {
                              return align_manipulator{
                                  boost::get<0>(pair),
                                  dataset::variant_view{boost::get<1>(pair)}};
                          });
    }
};

template <typename T>
auto row_value_constructor(const std::ostream& os, const T& rows)
{
    auto width = [&os](const auto& v) {
        std::ostringstream ss;
        ss.copyfmt(os);
        ss << v;
        return ss.str().size();
    };

    auto widths = back_constructor<std::vector<size_t>>(rows.columns(), width);
    for (const auto& row : rows)
        for (const auto& pair : boost::combine(widths, row))
            boost::get<0>(pair) =
                std::max<>(boost::get<0>(pair), width(boost::get<1>(pair)));

    return [widths = std::move(widths)](const auto& row) {
        return row_value_manipulator<decltype(row)>{widths, row};
    };
}

template <typename T>
size_t column_position(const T& rows, boost::string_view col_nm)
{
    const auto& cols = rows.columns();
    auto it = boost::range::find_if(
        cols,
        [col_nm, equal_to = unicode::case_insensitive_equal_to{}](auto& col) {
            return equal_to(col_nm, col);
        });
    if (it == cols.end())
        throw std::out_of_range(col_nm.to_string());
    return std::distance(cols.begin(), it);
}

}  // namespace detail

inline std::ostream& operator<<(std::ostream& os, const rowset& rows)
{
    boost::io::ios_flags_saver ifs(os);
    const auto& cols = rows.columns();
    auto line = std::vector<std::string>(cols.size());
    auto as_row = detail::row_value_constructor(os, rows);
    return os << std::setfill(' ') << as_row(cols) << "\n"
              << std::setfill('-') << as_row(line) << "\n"
              << std::setfill(' ') << list(rows, "\n", as_row);
}
template <typename ColumnNames>
rowset select(const ColumnNames& cols, const rowset& rows)
{
    auto positions = back_constructor<std::vector<size_t>>(
        cols,
        [&rows](auto& col) { return detail::column_position(rows, col); });

    ostream os;
    for (const auto& row : rows)
        for (auto pos : positions)
            os << row[pos];

    return {{std::begin(cols), std::end(cols)}, std::move(os).buf()};
}

}  // namespace dataset
}  // namespace bark

#endif  // BARK_DATASET_ROWSET_OPS_HPP
