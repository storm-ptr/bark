// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_DETAIL_LAYERS_HPP
#define BARK_DB_SLIPPY_DETAIL_LAYERS_HPP

#include <bark/db/provider.hpp>
#include <bark/db/slippy/detail/arcgis.hpp>
#include <bark/db/slippy/detail/bing.hpp>
#include <bark/db/slippy/detail/cartodb.hpp>
#include <bark/db/slippy/detail/double_gis.hpp>
#include <bark/db/slippy/detail/osm.hpp>
#include <bark/db/slippy/detail/rosreestr.hpp>
#include <bark/db/slippy/detail/sputnik.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/container.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <string>

namespace bark {
namespace db {
namespace slippy {
namespace detail {

class layers {
public:
    layer* operator[](const qualified_name& name)
    {
        layer* res = nullptr;
        boost::fusion::for_each(layers_, [&](auto& lr) {
            if (lr.name() == name)
                res = &lr;
        });
        return res ? res
                   : throw std::out_of_range(
                         boost::lexical_cast<std::string>(name));
    }

    layer_to_type_map dir()
    {
        layer_to_type_map res;
        boost::fusion::for_each(layers_, [&](auto& lr) {
            res.insert({lr.name(), layer_type::Raster});
        });
        return res;
    }

private:
    boost::fusion::list<arcgis_imagery,
                        arcgis_topo,
                        bing_aerials,
                        bing_maps,
                        cartodb,
                        double_gis,
                        osm,
                        rosreestr,
                        sputnik>
        layers_;
};

}  // namespace detail
}  // namespace slippy
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SLIPPY_DETAIL_LAYERS_HPP
