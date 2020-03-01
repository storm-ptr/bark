// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_OSM_HPP
#define BARK_DB_SLIPPY_OSM_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <sstream>

namespace bark::db::slippy {

struct osm : layer {
    qualified_name name() override { return id("osm"); }

    std::string url(const tile& tl) override
    {
        static random_index subdomain_{3};
        std::ostringstream os;
        os << "http://" << char('a' + subdomain_())
           << ".tile.openstreetmap.org/" << tl.z << "/" << tl.x << "/" << tl.y
           << ".png";
        return os.str();
    }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_OSM_HPP
