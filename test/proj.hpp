// Andrew Naplavkov

#ifndef BARK_TEST_PROJ_HPP
#define BARK_TEST_PROJ_HPP

#include <bark/geometry/as_text.hpp>
#include <bark/geometry/geom_from_wkb.hpp>
#include <bark/proj/transformer.hpp>
#include <bark/test/wkt.hpp>

TEST_CASE("proj")
{
    using namespace bark::geometry;
    using namespace bark::proj;

    /**
     * @see http://spatialreference.org/ref/epsg/4326/proj4/
     * @see http://spatialreference.org/ref/epsg/3395/proj4/
     */
    transformer latlong_to_mercator{
        "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs ",
        "+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 "
        "+units=m +no_defs "};

    for (auto&& wkt1 : Wkt) {
        auto wkb(as_binary(geom_from_text(wkt1)));
        latlong_to_mercator.inplace_forward(wkb);
        auto wkt2 = as_text(geom_from_wkb(wkb));
        CHECK(wkt1 != wkt2);
        latlong_to_mercator.inplace_backward(wkb);
        CHECK(wkt1 == as_text(geom_from_wkb(wkb)));
    }

    auto bbox = box{{0, 80}, {1, 81}};
    auto wkt = as_text(bbox);
    bbox = latlong_to_mercator.forward(bbox);
    CHECK(wkt != as_text(bbox));
    bbox = latlong_to_mercator.backward(bbox);
    CHECK(wkt == as_text(bbox));
}

#endif  // BARK_TEST_PROJ_HPP
