// Andrew Naplavkov

#ifndef BARK_PROJ_TRANSFORM_HPP
#define BARK_PROJ_TRANSFORM_HPP

#include <algorithm>
#include <bark/detail/grid.hpp>
#include <bark/geometry/geometry.hpp>
#include <cmath>
#include <iterator>
#include <proj_api.h>
#include <stdexcept>
#include <vector>

namespace bark::proj {

inline void transform(projPJ from, projPJ to, double* first, double* last)
{
    if (pj_is_latlong(from))
        std::transform(
            first, last, first, [](double v) { return v * DEG_TO_RAD; });

    auto size = std::distance(first, last);
    auto res =
        pj_transform(from, to, (long)size / 2, 2, first, first + 1, nullptr);
    if (res != 0)
        throw std::runtime_error(pj_strerrno(res));

    if (pj_is_latlong(to))
        std::transform(
            first, last, first, [](double v) { return v * RAD_TO_DEG; });
}

inline geometry::point transformed(projPJ from,
                                   projPJ to,
                                   const geometry::point& v)
{
    double coords[] = {v.x(), v.y()};
    transform(from, to, std::begin(coords), std::end(coords));
    return {coords[0], coords[1]};
}

inline geometry::box transformed(projPJ from, projPJ to, const geometry::box& v)
{
    grid gr(v, 32, 32);
    transform(from, to, gr.begin(), gr.end());
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

}  // namespace bark::proj

#endif  // BARK_PROJ_TRANSFORM_HPP
