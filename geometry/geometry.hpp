// Andrew Naplavkov

/**
 * @see
 * http://ibis.colostate.edu/WebContent/IBIS/BlueSpray/UsersGuide/GeometryClasses.png
 */

#ifndef BARK_GEOMETRY_GEOMETRY_HPP
#define BARK_GEOMETRY_GEOMETRY_HPP

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>
#include <boost/variant.hpp>
#include <vector>

namespace bark {
namespace geometry {

using point = boost::geometry::model::d2::point_xy<double>;
using linestring = boost::geometry::model::linestring<point>;
using polygon = boost::geometry::model::polygon<point, false, true>;
using multi_point = boost::geometry::model::multi_point<point>;
using multi_linestring = boost::geometry::model::multi_linestring<linestring>;
using multi_polygon = boost::geometry::model::multi_polygon<polygon>;
struct geometry_collection;

using geometry = boost::variant<point,
                                linestring,
                                polygon,
                                multi_point,
                                multi_linestring,
                                multi_polygon,
                                boost::recursive_wrapper<geometry_collection> >;

struct geometry_collection : std::vector<geometry> {
};

using box = boost::geometry::model::box<point>;
using multi_box = std::vector<box>;

struct view {
    box extent;
    double scale;
};

}  // namespace geometry
}  // namespace bark

#endif  // BARK_GEOMETRY_GEOMETRY_HPP
