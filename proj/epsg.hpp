// Andrew Naplavkov

#ifndef BARK_PROJ_EPSG_HPP
#define BARK_PROJ_EPSG_HPP

#include <bark/db/sqlite/command.hpp>
#include <bark/proj/bimap.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace bark::proj {

/// Applied geodesy organization
inline const bimap& epsg()
{
    static const bimap singleton = [] {
        using namespace db;
        enum columns { Srid, Proj4 };
        auto res = bimap{};
        auto cmd = sqlite::command{":memory:"};
        exec(cmd, "SELECT InitSpatialMetaData(1)");
        auto bld = builder(cmd);
        bld << "SELECT auth_srid, proj4text FROM spatial_ref_sys"
               " WHERE LOWER(srtext) NOT LIKE "
            << param{"%deprecated%"}
            << " AND LOWER(auth_name) = " << param{"epsg"}
            << " AND auth_srid IS NOT NULL AND proj4text IS NOT NULL";
        exec(cmd, bld);
        auto rows = fetch_all(cmd);
        for (auto& row : select(rows))
            res.insert(boost::lexical_cast<int>(row[Srid]),
                       boost::lexical_cast<std::string>(row[Proj4]));

        /// @see http://spatialreference.org/ref/epsg/4326/proj4/
        res.insert(4326, "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        /// @see
        /// http://spatialreference.org/ref/sr-org/epsg3857-wgs84-web-mercator-auxiliary-sphere/proj4/
        res.insert(
            3857,
            "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 "
            "+y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs");
        return res;
    }();
    return singleton;
}

inline std::string abbreviation(const std::string& pj)
try {
    return "EPSG:" + std::to_string(epsg().find_srid(pj));
}
catch (const std::out_of_range&) {
    return boost::trim_copy(pj);
}

}  // namespace bark::proj

#endif  // BARK_PROJ_EPSG_HPP
