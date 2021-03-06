// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_LAYER_HPP
#define BARK_DB_SLIPPY_LAYER_HPP

#include <bark/db/qualified_name.hpp>
#include <bark/db/slippy/detail/tile.hpp>
#include <bark/proj/epsg.hpp>
#include <bark/proj/transformer.hpp>
#include <string>

namespace bark::db::slippy {

constexpr int LayerEpsg = 3857;
constexpr int TileEpsg = 4326;

struct layer {
    virtual ~layer() = default;
    virtual qualified_name name() = 0;
    virtual std::string url(const tile&) = 0;
    virtual int zmax() { return 19; };
};

inline std::string projection()
{
    return proj::epsg().find_proj(LayerEpsg);
}

inline proj::transformer tile_to_layer_transformer()
{
    return {proj::epsg().find_proj(TileEpsg), projection()};
}

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_LAYER_HPP
