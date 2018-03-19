// Andrew Naplavkov

#ifndef BARK_PROJ_PRINT_HPP
#define BARK_PROJ_PRINT_HPP

#include <bark/proj/epsg.hpp>
#include <boost/algorithm/string.hpp>

namespace bark {
namespace proj {

inline std::string print(const std::string& pj) try {
    return "EPSG:" + std::to_string(epsg().find_srid(pj));
}
catch (const std::out_of_range&) {
    return boost::trim_copy(pj);
}

}  // namespace proj
}  // namespace bark

#endif  // BARK_PROJ_PRINT_HPP
