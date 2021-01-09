// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_BING_HPP
#define BARK_DB_SLIPPY_BING_HPP

#include <bark/db/slippy/detail/layer.hpp>

namespace bark::db::slippy {

/// @see https://msdn.microsoft.com/en-us/library/bb259689.aspx
struct bing_quad_key {
    const tile& tl;

    friend std::ostream& operator<<(std::ostream& os, const bing_quad_key& that)
    {
        for (auto i = that.tl.z; i > 0; i--) {
            char digit = '0';
            int mask = 1 << (i - 1);
            if ((that.tl.x & mask) != 0) {
                digit++;
            }
            if ((that.tl.y & mask) != 0) {
                digit++;
                digit++;
            }
            os << digit;
        }
        return os;
    }
};

struct bing_maps : layer {
    qualified_name name() override { return id("bing", "maps"); }

    std::string url(const tile& tl) override
    {
        return concat("http://tiles.virtualearth.net/tiles/r",
                      bing_quad_key{tl},
                      ".png?g=1&shading=hill&n=z");
    }
};

struct bing_aerials : layer {
    qualified_name name() override { return id("bing", "aerials"); }

    std::string url(const tile& tl) override
    {
        return concat("http://tiles.virtualearth.net/tiles/a",
                      bing_quad_key{tl},
                      ".jpeg?g=1");
    }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_BING_HPP
