// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_CARTODB_HPP
#define BARK_DB_SLIPPY_CARTODB_HPP

#include <bark/db/slippy/detail/layer.hpp>

namespace bark::db::slippy {

struct cartodb : layer {
    qualified_name name() override { return id("cartodb"); }

    std::string url(const tile& tl) override
    {
        return concat("http://basemaps.cartocdn.com/light_all/",
                      tl.z,
                      "/",
                      tl.x,
                      "/",
                      tl.y,
                      ".png");
    }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_CARTODB_HPP
