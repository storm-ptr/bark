// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DETAIL_BING_HPP
#define BARK_DB_SLIPPY_DETAIL_BING_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <bark/detail/random_index.hpp>
#include <sstream>

namespace bark {
namespace db {
namespace slippy {
namespace detail {

/// @see https://msdn.microsoft.com/en-us/library/bb259689.aspx
struct quad_key_manipulator {
    const tile& tl;

    friend std::ostream& operator<<(std::ostream& os,
                                    const quad_key_manipulator& that)
    {
        for (auto i = that.tl.z; i > 0; i--) {
            char digit = '0';
            const int mask = 1 << (i - 1);
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

inline quad_key_manipulator quad_key(const tile& tl)
{
    return {tl};
}

class bing_maps : public layer {
    random_index subdomain_{8};

public:
    qualified_name name() override { return id("bing", "maps"); }

    int zmax() override { return 19; }

    std::string url(const tile& tl) override
    {
        // &mkt=en-US
        std::ostringstream os;
        os << "http://ecn.t" << char('0' + subdomain_())
           << ".tiles.virtualearth.net/tiles/r" << quad_key(tl)
           << ".png?g=1&shading=hill&n=z";
        return os.str();
    }
};

class bing_aerials : public layer {
    random_index subdomain_{8};

public:
    qualified_name name() override { return id("bing", "aerials"); }

    int zmax() override { return 19; }

    std::string url(const tile& tl) override
    {
        // &mkt=en-US
        std::ostringstream os;
        os << "http://ecn.t" << char('0' + subdomain_())
           << ".tiles.virtualearth.net/tiles/a" << quad_key(tl) << ".jpeg?g=1";
        return os.str();
    }
};

}  // namespace detail
}  // namespace slippy
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SLIPPY_DETAIL_BING_HPP
