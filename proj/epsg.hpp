// Andrew Naplavkov

#ifndef BARK_PROJ_EPSG_HPP
#define BARK_PROJ_EPSG_HPP

#include <bark/db/sqlite/command.hpp>
#include <bark/proj/bimap.hpp>
#include <boost/lexical_cast.hpp>

namespace bark {
namespace proj {

inline const bimap& epsg()
{
    static const bimap singleton = [] {
        using namespace db;
        bimap res;
        sqlite::command cmd{":memory:"};
        exec(cmd, "SELECT InitSpatialMetaData(1)");
        auto bld = builder(cmd);
        bld << "SELECT auth_srid, proj4text FROM spatial_ref_sys"
               " WHERE LOWER(srtext) NOT LIKE "
            << param("%deprecated%")
            << " AND LOWER(auth_name) = " << param("epsg")
            << " AND auth_srid IS NOT NULL AND proj4text IS NOT NULL";
        exec(cmd, bld);
        for (auto& row : fetch_all(cmd))
            res.insert(boost::lexical_cast<int>(row[0]),
                       boost::lexical_cast<std::string>(row[1]));

        /**
         * @see http://spatialreference.org/ref/epsg/4326/proj4/
         * @see
         * http://spatialreference.org/ref/sr-org/epsg3857-wgs84-web-mercator-auxiliary-sphere/proj4/
         */
        res.insert(4326, "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        res.insert(
            3857,
            "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 "
            "+y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs");
        return res;
    }();
    return singleton;
}

}  // namespace proj
}  // namespace bark

#endif  // BARK_PROJ_EPSG_HPP
