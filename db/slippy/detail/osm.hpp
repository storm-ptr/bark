// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DETAIL_OSM_HPP
#define BARK_DB_SLIPPY_DETAIL_OSM_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <sstream>

namespace bark::db::slippy::detail {

struct osm : layer {
    qualified_name name() override { return id("osm"); }

    int zmax() override { return 19; }

    std::string url(const tile& tl) override
    {
        std::ostringstream os;
        os << "http://tile.openstreetmap.org/" << tl.z << "/" << tl.x << "/"
           << tl.y << ".png";
        return os.str();
    }
};

}  // namespace bark::db::slippy::detail

#endif  // BARK_DB_SLIPPY_DETAIL_OSM_HPP
