// Andrew Naplavkov

#ifndef BARK_TEST_RASTER_HPP
#define BARK_TEST_RASTER_HPP

#include <bark/db/gdal/provider.hpp>
#include <iostream>

TEST_CASE("gdal_raster")
{
    using namespace bark;
    using namespace bark::db;

    gdal::provider pvd(R"(./data/albers27.tif)");
    auto reg = pvd.dir();
    REQUIRE(!reg.empty());
    auto [layer, type] = *reg.begin();
    REQUIRE(type == bark::db::meta::layer_type::Raster);
    std::cout << layer << "\n"
              << pvd.projection(layer) << "\nBOX"
              << boost::geometry::dsv(pvd.extent(layer)) << std::endl;
}

#endif  // BARK_TEST_RASTER_HPP
