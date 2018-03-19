// Andrew Naplavkov

#ifndef BARK_DB_GDAL_DETAIL_FRAME_HPP
#define BARK_DB_GDAL_DETAIL_FRAME_HPP

#include <bark/db/gdal/detail/common.hpp>
#include <bark/geometry/geometry_ops.hpp>

namespace bark {
namespace db {
namespace gdal {
namespace detail {

/**
 * coefficients for transforming between pixel/line raster space, and projection
 * coordinates (X,Y) space
 * @see http://www.gdal.org/gdal_tutorial.html
 */
class frame {
public:
    explicit frame(GDALDatasetH dataset)
        : pixels_{GDALGetRasterXSize(dataset)}
        , lines_{GDALGetRasterYSize(dataset)}
    {
        check(GDALGetGeoTransform(dataset, affine_));
    }

    geometry::box extent() const
    {
        return backward({{0., 0.}, {(double)pixels_, (double)lines_}});
    }

    geometry::box pixel() const
    {
        auto pixel = (double)(pixels_ / 2);
        auto line = (double)(lines_ / 2);
        return backward({{pixel, line}, {pixel + 1, line + 1}});
    }

private:
    int pixels_;
    int lines_;
    double affine_[6];

    double to_x(double pixel, double line) const
    {
        return affine_[0] + pixel * affine_[1] + line * affine_[2];
    }

    double to_y(double pixel, double line) const
    {
        return affine_[3] + pixel * affine_[4] + line * affine_[5];
    }

    geometry::box backward(const geometry::box& ext) const
    {
        auto min_pixel = geometry::left(ext);
        auto min_line = geometry::bottom(ext);
        auto max_pixel = geometry::right(ext);
        auto max_line = geometry::top(ext);
        auto res = geometry::box{
            {to_x(min_pixel, min_line), to_y(min_pixel, min_line)},
            {to_x(max_pixel, max_line), to_y(max_pixel, max_line)}};
        boost::geometry::correct(res);
        return res;
    }
};

}  // namespace detail
}  // namespace gdal
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_GDAL_DETAIL_FRAME_HPP
