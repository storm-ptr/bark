// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_SPUTNIK_HPP
#define BARK_DB_SLIPPY_SPUTNIK_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <sstream>

namespace bark::db::slippy {

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

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_SPUTNIK_HPP
