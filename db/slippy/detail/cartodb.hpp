// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DETAIL_CARTODB_HPP
#define BARK_DB_SLIPPY_DETAIL_CARTODB_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <bark/detail/random_index.hpp>
#include <sstream>

namespace bark::db::slippy::detail {

class cartodb : public layer {
    random_index subdomain_{4};

public:
    qualified_name name() override { return id("cartodb"); }

    int zmax() override { return 19; }

    std::string url(const tile& tl) override
    {
        std::ostringstream os;
        os << "http://cartodb-basemaps-" << char('a' + subdomain_())
           << ".global.ssl.fastly.net/light_all/" << tl.z << "/" << tl.x << "/"
           << tl.y << ".png";
        return os.str();
    }
};

}  // namespace bark::db::slippy::detail

#endif  // BARK_DB_SLIPPY_DETAIL_CARTODB_HPP
