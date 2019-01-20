// Andrew Naplavkov

#ifndef BARK_TEST_GEOMETRY_HPP
#define BARK_TEST_GEOMETRY_HPP

#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/as_text.hpp>
#include <bark/geometry/geom_from_text.hpp>
#include <bark/geometry/geom_from_wkb.hpp>
#include <bark/test/wkt.hpp>

TEST_CASE("geometry")
{
    using namespace bark::geometry;

    for (auto&& wkt1 : Wkt) {
        REQUIRE(as_text(geom_from_text(wkt1)) == wkt1);
        auto wkt2 = as_text(geom_from_wkb(as_binary(geom_from_text(wkt1))));
        auto wkt3 = as_text(geom_from_wkb(as_binary(geom_from_text(wkt2))));
        REQUIRE(wkt1 == wkt3);
    }

    REQUIRE(
        "POLYGON((-90 21,-89.5 21,-89.5 21.5,-90 21.5,-90 21))" ==
        as_text(geom_from_wkb(as_binary(box{{-90.0, 21.0}, {-89.5, 21.5}}))));
}

#endif  // BARK_TEST_GEOMETRY_HPP
