// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DETAIL_OSM_HPP
#define BARK_DB_SLIPPY_DETAIL_OSM_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <bark/detail/random_index.hpp>
#include <sstream>

namespace bark {
namespace db {
namespace slippy {
namespace detail {

class osm : public layer {
    random_index subdomain_{3};

public:
    qualified_name name() override { return id("osm"); }

    int zmax() override { return 19; }

    std::string url(const tile& tl) override
    {
        std::ostringstream os;
        os << "http://" << char('a' + subdomain_())
           << ".tile.openstreetmap.org/" << tl.z << "/" << tl.x << "/" << tl.y
           << ".png";
        return os.str();
    }
};

}  // namespace detail
}  // namespace slippy
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SLIPPY_DETAIL_OSM_HPP
