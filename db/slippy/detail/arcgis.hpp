// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_ARCGIS_HPP
#define BARK_DB_SLIPPY_ARCGIS_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <sstream>

namespace bark::db::slippy {

class arcgis_imagery : public layer {
public:
    qualified_name name() override { return id("arcgis", "imagery"); }

    int zmax() override { return 17; }

    std::string url(const tile& tl) override
    {
        std::ostringstream os;
        os << "http://services.arcgisonline.com/ArcGIS/rest/services/"
              "World_Imagery/MapServer/tile/"
           << tl.z << "/" << tl.y << "/" << tl.x << ".png";
        return os.str();
    }
};

class arcgis_topo : public layer {
public:
    qualified_name name() override { return id("arcgis", "topo"); }

    int zmax() override { return 19; }

    std::string url(const tile& tl) override
    {
        std::ostringstream os;
        os << "http://services.arcgisonline.com/ArcGIS/rest/services/"
              "World_Topo_Map/MapServer/tile/"
           << tl.z << "/" << tl.y << "/" << tl.x << ".png";
        return os.str();
    }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_ARCGIS_HPP
