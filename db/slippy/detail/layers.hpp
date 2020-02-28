// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_LAYERS_HPP
#define BARK_DB_SLIPPY_LAYERS_HPP

#include <bark/db/provider.hpp>
#include <bark/db/slippy/detail/arcgis.hpp>
#include <bark/db/slippy/detail/bing.hpp>
#include <bark/db/slippy/detail/cartodb.hpp>
#include <bark/db/slippy/detail/double_gis.hpp>
#include <bark/db/slippy/detail/google.hpp>
#include <bark/db/slippy/detail/osm.hpp>
#include <bark/db/slippy/detail/sputnik.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/container.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <string>

namespace bark::db::slippy {

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

    std::map<qualified_name, layer_type> dir()
    {
        std::map<qualified_name, layer_type> res;
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
                        google_hybrid,
                        google_map,
                        google_satellite,
                        osm,
                        sputnik>
        layers_;
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_LAYERS_HPP
