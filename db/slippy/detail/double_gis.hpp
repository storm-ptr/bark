// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DOUBLE_GIS_HPP
#define BARK_DB_SLIPPY_DOUBLE_GIS_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <sstream>

namespace bark::db::slippy {

struct double_gis : layer {
    qualified_name name() override { return id("2gis"); }

    std::string url(const tile& tl) override
    {
        static random_index subdomain_{4};
        std::ostringstream os;
        os << "http://tile" << char('0' + subdomain_())
           << ".maps.2gis.com/tiles?x=" << tl.x << "&y=" << tl.y
           << "&z=" << tl.z;
        return os.str();
    }

    int zmax() override { return 18; }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_DOUBLE_GIS_HPP
