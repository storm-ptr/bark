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

class cell_visitor : public boost::static_visitor<void> {
    std::ostream& os_;
    size_t width_;

public:
    cell_visitor(std::ostream& os, size_t width) : os_(os), width_(width) {}

    void operator()(boost::blank) const { os_ << std::setw(width_) << ""; }

    template <typename T>
    void operator()(T v) const
    {
        os_ << std::setw(width_) << std::right << v;
    }

    void operator()(string_view v) const
    {
        os_ << std::setw(width_) << std::left << v;
    }
};

struct cell_manipulator {
    variant_view view;
    size_t width;

    friend std::ostream& operator<<(std::ostream& os,
                                    const cell_manipulator& manip)
    {
        os << std::setw(1) << "";
        boost::apply_visitor(cell_visitor{os, manip.width}, manip.view);
        return os << std::setw(1) << "";
    }
};

template <typename Row>
struct row_manipulator {
    const Row& row;
    const std::vector<size_t>& widths;

    friend std::ostream& operator<<(std::ostream& os,
                                    const row_manipulator& manip)
    {
        return os << list(boost::combine(manip.row, manip.widths),
                          "|",
                          [](const auto& pair) {
                              return cell_manipulator{
                                  dataset::variant_view{boost::get<0>(pair)},
                                  boost::get<1>(pair)};
                          });
    }
};

template <typename T>
auto row_value_constructor(const std::ostream& os, const T& rows)
{
    auto width = [&](const auto& v) {
        std::ostringstream ss;
        ss.copyfmt(os);
        ss << v;
        return ss.str().size();
    };
    auto widths = back_constructor<std::vector<size_t>>(rows.columns(), width);
    for (const auto& row : rows)
        for (const auto& pair : boost::combine(row, widths))
            boost::get<1>(pair) =
                std::max<>(width(boost::get<0>(pair)), boost::get<1>(pair));
    return [=](const auto& row) {
        return detail::row_manipulator<decltype(row)>{row, widths};
    };
}

template <typename ColumnNames>
size_t column_position(const ColumnNames& cols, string_view col)
{
    auto it = boost::range::find_if(cols, [&](const auto& item) {
        return unicode::case_insensitive_equal_to{}(col, item);
    });
    if (it == cols.end())
        throw std::out_of_range(col.to_string());
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
    auto positions =
        back_constructor<std::vector<size_t>>(cols, [&](const auto& col) {
            return detail::column_position(rows.columns(), col);
        });

    ostream os;
    for (const auto& row : rows)
        for (auto pos : positions)
            os << row[pos];

    return {{std::begin(cols), std::end(cols)}, std::move(os).buf()};
}

}  // namespace dataset
}  // namespace bark

#endif  // BARK_DATASET_ROWSET_OPS_HPP
