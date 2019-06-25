// Andrew Naplavkov

#ifndef BARK_PROJ_ABBREVIATION_HPP
#define BARK_PROJ_ABBREVIATION_HPP

#include <bark/proj/epsg.hpp>
#include <boost/algorithm/string.hpp>

namespace bark::proj {

inline std::string abbreviation(const std::string& pj) try {
    return "EPSG:" + std::to_string(epsg().find_srid(pj));
}
catch (const std::out_of_range&) {
    return boost::trim_copy(pj);
}

}  // namespace bark::proj

#endif  // BARK_PROJ_ABBREVIATION_HPP