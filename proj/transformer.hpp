// Andrew Naplavkov

#ifndef BARK_PROJ_TRANSFORMER_HPP
#define BARK_PROJ_TRANSFORMER_HPP

#include <algorithm>
#include <bark/detail/grid.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/detail/stream.hpp>
#include <cmath>
#include <iterator>
#include <vector>

namespace bark::proj {

/// Performs conversions between cartographic projections.

/// PROJ.4 library wrapper.
/// @see https://en.wikipedia.org/wiki/PROJ
class transformer {
public:
    /// @param from is a source PROJ.4 string;
    /// @param to is a target PROJ.4 string.
    transformer(const std::string& from, const std::string& to)
        : tf_{from, to}, is_trivial_{from == to}
    {
    }

    bool is_trivial() const { return is_trivial_; }

    geometry::point forward(const geometry::point& val) const
    {
        return trans(PJ_FWD, val);
    }

    geometry::point backward(const geometry::point& val) const
    {
        return trans(PJ_INV, val);
    }

    geometry::box forward(const geometry::box& val) const
    {
        return trans(PJ_FWD, val);
    }

    geometry::box backward(const geometry::box& val) const
    {
        return trans(PJ_INV, val);
    }

    void inplace_forward(double* first, double* last) const
    {
        tf_.trans_generic(PJ_FWD, first, last);
    }

    void inplace_backward(double* first, double* last) const
    {
        tf_.trans_generic(PJ_INV, first, last);
    }

    void inplace_forward(blob_view wkb) const { stream{tf_, PJ_FWD}(wkb); }

    void inplace_backward(blob_view wkb) const { stream{tf_, PJ_INV}(wkb); }

    auto inplace_forward() const
    {
        return [this](auto... args) { inplace_forward(args...); };
    }

    auto inplace_backward() const
    {
        return [this](auto... args) { inplace_backward(args...); };
    }

private:
    transformation tf_;
    bool is_trivial_;

    geometry::point trans(PJ_DIRECTION dir, const geometry::point& val) const
    {
        double coords[] = {val.x(), val.y()};
        tf_.trans_generic(dir, std::begin(coords), std::end(coords));
        return {coords[0], coords[1]};
    }

    geometry::box trans(PJ_DIRECTION dir, const geometry::box& val) const
    {
        grid gr(val, 32, 32);
        tf_.trans_generic(dir, gr.begin(), gr.end());
        auto points = gr.rows() * gr.cols();
        std::vector<double> xs, ys;
        xs.reserve(points);
        ys.reserve(points);

        for (size_t row = 0; row < gr.rows(); ++row)
            for (size_t col = 0; col < gr.cols(); ++col) {
                auto x = gr.x(row, col);
                auto y = gr.y(row, col);
                if (std::isfinite(x))
                    xs.push_back(x);
                if (std::isfinite(y))
                    ys.push_back(y);
            }
        if (xs.empty() || ys.empty())
            throw std::runtime_error("transformed box");
        auto minmax_x = std::minmax_element(xs.begin(), xs.end());
        auto minmax_y = std::minmax_element(ys.begin(), ys.end());
        return {{*minmax_x.first, *minmax_y.first},
                {*minmax_x.second, *minmax_y.second}};
    }
};

}  // namespace bark::proj

#endif  // BARK_PROJ_TRANSFORMER_HPP
