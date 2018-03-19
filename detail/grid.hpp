// Andrew Naplavkov

#ifndef BARK_DETAIL_GRID_HPP
#define BARK_DETAIL_GRID_HPP

#include <bark/geometry/geometry_ops.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/sum_kahan.hpp>
#include <boost/multi_array.hpp>

namespace bark {
namespace detail {

/// Contiguous XY-coordinates container
class grid {
public:
    using value_type = double;
    using sequence_type = boost::multi_array<value_type, 3>;
    using kahan_accumulator = boost::accumulators::accumulator_set<
        value_type,
        boost::accumulators::stats<boost::accumulators::tag::sum_kahan>>;

    grid(const geometry::box& ext, size_t rows, size_t cols)
        : data_{boost::extents[rows][cols][2]}
    {
        if (rows < 2 || cols < 2)
            std::fill(begin(), end(), NAN);
        else {
            auto dx = geometry::width(ext) / (cols - 1);
            auto dy = geometry::height(ext) / (rows - 1);
            auto min_x = geometry::left(ext);
            auto min_y = geometry::bottom(ext);
            kahan_accumulator acc_y(min_y);
            for (size_t row = 0; row < rows; ++row, acc_y(dy)) {
                kahan_accumulator acc_x(min_x);
                for (size_t col = 0; col < cols; ++col, acc_x(dx)) {
                    x(row, col) = boost::accumulators::sum_kahan(acc_x);
                    y(row, col) = boost::accumulators::sum_kahan(acc_y);
                }
            }
        }
    }

    size_t rows() const { return data_.shape()[0]; }
    size_t cols() const { return data_.shape()[1]; }
    value_type* begin() { return data_.data(); }
    value_type* end() { return data_.data() + data_.num_elements(); }
    value_type& x(size_t row, size_t col) { return data_[row][col][0]; }
    value_type& y(size_t row, size_t col) { return data_[row][col][1]; }

private:
    sequence_type data_;
};

}  // namespace detail

using grid = detail::grid;

}  // namespace bark

#endif  // BARK_DETAIL_GRID_HPP
