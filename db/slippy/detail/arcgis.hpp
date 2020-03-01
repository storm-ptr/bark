// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_ARCGIS_HPP
#define BARK_DB_SLIPPY_ARCGIS_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <sstream>

namespace bark::db::slippy {

inline std::string arcgis_url(std::string_view lyr, const tile& tl)
{
    std::ostringstream os;
    os << "http://services.arcgisonline.com/ArcGIS/rest/services/World_" << lyr
       << "/MapServer/tile/" << tl.z << "/" << tl.y << "/" << tl.x << ".png";
    return os.str();
}

struct arcgis_imagery : layer {
    qualified_name name() override { return id("arcgis", "imagery"); }

    std::string url(const tile& tl) override
    {
        return arcgis_url("Imagery", tl);
    }

    int zmax() override { return 17; }
};

struct arcgis_topo_map : layer {
    qualified_name name() override { return id("arcgis", "topo_map"); }

    std::string url(const tile& tl) override
    {
        return arcgis_url("Topo_Map", tl);
    }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_ARCGIS_HPP
