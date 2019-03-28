// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DOUBLE_GIS_HPP
#define BARK_DB_SLIPPY_DOUBLE_GIS_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <sstream>

namespace bark::db::slippy {

class double_gis : public layer {
    random_index subdomain_{4};

public:
    qualified_name name() override { return id("2gis"); }

    int zmax() override { return 18; }

    std::string url(const tile& tl) override
    {
        std::ostringstream os;
        os << "http://tile" << char('0' + subdomain_())
           << ".maps.2gis.com/tiles?x=" << tl.x << "&y=" << tl.y
           << "&z=" << tl.z;
        return os.str();
    }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_DOUBLE_GIS_HPP
