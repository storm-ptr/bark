// Andrew Naplavkov

#ifndef BARK_PROJ_DETAIL_TRANSFORM_HPP
#define BARK_PROJ_DETAIL_TRANSFORM_HPP

#include <algorithm>
#include <bark/detail/grid.hpp>
#include <bark/geometry/geometry.hpp>
#include <cmath>
#include <iterator>
#include <proj_api.h>
#include <stdexcept>
#include <vector>

namespace bark {
namespace proj {
namespace detail {

inline void transform(projPJ pj_from, projPJ pj_to, double* begin, double* end)
{
    if (pj_is_latlong(pj_from))
        std::transform(
            begin, end, begin, [](double coord) { return coord * DEG_TO_RAD; });

    auto size = std::distance(begin, end);
    auto res = pj_transform(
        pj_from, pj_to, (long)size / 2, 2, begin, begin + 1, nullptr);
    if (res != 0)
        throw std::runtime_error(pj_strerrno(res));

    if (pj_is_latlong(pj_to))
        std::transform(
            begin, end, begin, [](double coord) { return coord * RAD_TO_DEG; });
}

inline geometry::point transformed(projPJ pj_from,
                                   projPJ pj_to,
                                   const geometry::point& point)
{
    double coords[] = {point.x(), point.y()};
    transform(pj_from, pj_to, std::begin(coords), std::end(coords));
    return {coords[0], coords[1]};
}

inline geometry::box transformed(projPJ pj_from,
                                 projPJ pj_to,
                                 const geometry::box& box)
{
    grid gr(box, 32, 32);
    transform(pj_from, pj_to, gr.begin(), gr.end());
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
        throw std::runtime_error("transform box error");
    auto minmax_x = std::minmax_element(xs.begin(), xs.end());
    auto minmax_y = std::minmax_element(ys.begin(), ys.end());
    return {{*minmax_x.first, *minmax_y.first},
            {*minmax_x.second, *minmax_y.second}};
}

}  // namespace detail
}  // namespace proj
}  // namespace bark

#endif  // BARK_PROJ_DETAIL_TRANSFORM_HPP
