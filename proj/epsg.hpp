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
        return res;
    }();
    return singleton;
}

}  // namespace proj
}  // namespace bark

#endif  // BARK_PROJ_EPSG_HPP
