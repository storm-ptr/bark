// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DETAIL_ROSREESTR_HPP
#define BARK_DB_SLIPPY_DETAIL_ROSREESTR_HPP

#include <bark/db/slippy/detail/layer.hpp>
#include <bark/detail/random_index.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <limits>
#include <locale>
#include <sstream>

namespace bark {
namespace db {
namespace slippy {
namespace detail {

class rosreestr : public layer {
public:
    qualified_name name() override { return id("rosreestr"); }

    int zmax() override { return 20; }

    std::string url(const tile& tl) override
    {
        using geometry::operator<<;
        auto tf = tile_to_layer_transformer();
        auto ext = tf.forward(extent(tl));
        std::ostringstream os;
        os.imbue(std::locale::classic());
        os.precision(std::numeric_limits<double>::max_digits10);
        os << "http://pkk5.rosreestr.ru/arcgis/rest/services/Cadastre/Cadastre/"
              "MapServer/export?format=PNG32&f=image&transparent=true&imageSR="
           << LayerEpsg << "&size=" << Pixels << "," << Pixels
           << "&bboxSR=" << LayerEpsg << "&bbox=" << ext;
        return os.str();
    }
};

}  // namespace detail
}  // namespace slippy
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SLIPPY_DETAIL_ROSREESTR_HPP
