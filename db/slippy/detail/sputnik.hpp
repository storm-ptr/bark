// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_SPUTNIK_HPP
#define BARK_DB_SLIPPY_SPUTNIK_HPP

#include <bark/db/slippy/detail/layer.hpp>

namespace bark::db::slippy {

struct sputnik : layer {
    qualified_name name() override { return id("sputnik"); }

    std::string url(const tile& tl) override
    {
        return concat(
            "http://tilessputnik.ru/", tl.z, "/", tl.x, "/", tl.y, ".png");
    }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_SPUTNIK_HPP
