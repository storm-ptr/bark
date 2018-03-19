// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DETAIL_SPUTNIK_HPP
#define BARK_DB_SLIPPY_DETAIL_SPUTNIK_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <sstream>

namespace bark {
namespace db {
namespace slippy {
namespace detail {

class sputnik : public layer {
public:
    qualified_name name() override { return id("sputnik"); }

    int zmax() override { return 19; }

    std::string url(const tile& tl) override
    {
        std::ostringstream os;
        os << "http://tilessputnik.ru/" << tl.z << "/" << tl.x << "/" << tl.y
           << ".png";
        return os.str();
    }
};

}  // namespace detail
}  // namespace slippy
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SLIPPY_DETAIL_SPUTNIK_HPP
