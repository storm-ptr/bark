// Andrew Naplavkov

#ifndef BARK_PROJ_NORMALIZE_HPP
#define BARK_PROJ_NORMALIZE_HPP

#include <bark/proj/detail/projection.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <string>

namespace bark::proj {

inline std::string normalize(const std::string& pj) try {
    return pj.empty() ? std::string{}
                      : boost::trim_copy(detail::projection{pj}.get_def());
}
catch (const std::exception&) {
    return boost::trim_copy(pj);
}

}  // namespace bark::proj

#endif  // BARK_PROJ_NORMALIZE_HPP
