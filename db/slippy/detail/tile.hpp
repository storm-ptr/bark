// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DETAIL_TILE_HPP
#define BARK_DB_SLIPPY_DETAIL_TILE_HPP

#include <algorithm>
#include <array>
#include <bark/geometry/geometry_ops.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <cmath>

namespace bark::db::slippy::detail {

constexpr int Pixels = 256;

/// @see http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
struct tile {
    int x = 0;
    int y = 0;
    int z = 0;
};

using tiles = std::vector<tile>;

inline auto sub(const tile& tl)
{
    std::array<tile, 4> res;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            res[i * 2 + j] = {2 * tl.x + i, 2 * tl.y + j, tl.z + 1};
    return res;
}

inline double left(const tile& tl)
{
    return tl.x / std::pow(2, tl.z) * 360 - 180;
}

inline double top(const tile& tl)
{
    constexpr auto Pi = boost::math::constants::pi<double>();
    auto n = Pi - 2 * Pi * tl.y / std::pow(2, tl.z);
    return 180 / Pi * atan((exp(n) - exp(-n)) / 2);
}

inline geometry::box extent(const tile& tl)
{
    return {{left(tl), top({tl.x, tl.y + 1, tl.z})},
            {left({tl.x + 1, tl.y, tl.z}), top(tl)}};
}

inline geometry::box pixel(const tile& tl)
{
    auto ext = extent(tl);
    auto center = boost::geometry::return_centroid<geometry::point>(ext);
    return {center,
            {center.x() + geometry::width(ext) / Pixels,
             center.y() + geometry::height(ext) / Pixels}};
}

template <class Predicate>
void depth_first_search(tiles& tls, const tile& tl, Predicate filter)
{
    if (!filter(tl))
        return;
    tls.push_back(tl);
    for (auto& sub_tl : sub(tl))
        depth_first_search(tls, sub_tl, filter);
}

template <class Predicate>
tiles depth_first_search(Predicate filter)
{
    tiles tls;
    depth_first_search(tls, {}, filter);
    return tls;
}

inline tile match(const geometry::box& px, int zmax)
{
    using namespace boost::geometry;

    auto px_area = area(px);
    auto sub_px_area = area(geometry::sub(px).front());

    auto tiny = [&](const tile& tl) {
        return tl.z > zmax || area(pixel(tl)) < sub_px_area;
    };
    auto filter = [&](const tile& tl) {
        return intersects(px, extent(tl)) && !tiny(tl);
    };
    auto diff = [&](const tile& tl) {
        return std::fabs(area(pixel(tl)) - px_area);
    };
    auto cmp = [&](const tile& lhs, const tile& rhs) {
        return diff(lhs) < diff(rhs);
    };

    auto tls = depth_first_search(filter);
    return tls.empty() ? tile{}
                       : *std::min_element(tls.begin(), tls.end(), cmp);
}

inline tiles tile_coverage(const geometry::box& ext, int z)
{
    auto filter = [&](const tile& tl) {
        if (tl.z > z || !boost::geometry::intersects(ext, extent(tl)))
            return false;
        if (tl.z < z)
            return true;
        geometry::box visible{};
        boost::geometry::intersection(ext, extent(tl), visible);
        auto sub_pixel = pixel(sub(tl).front());
        return geometry::width(visible) > geometry::width(sub_pixel) &&
               geometry::height(visible) > geometry::height(sub_pixel);
    };

    auto tls = depth_first_search(filter);
    boost::remove_erase_if(tls, [&](const tile& tl) { return tl.z != z; });
    return tls;
}

}  // namespace bark::db::slippy::detail

#endif  // BARK_DB_SLIPPY_DETAIL_TILE_HPP
