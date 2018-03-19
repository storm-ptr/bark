// Andrew Naplavkov

#ifndef BARK_PROJ_NORMALIZE_HPP
#define BARK_PROJ_NORMALIZE_HPP

#include <bark/proj/detail/projection.hpp>
#include <string>

namespace bark {
namespace proj {

inline std::string normalize(const std::string& pj)
{
    return pj.empty() ? std::string{} : detail::projection{pj}.get_def();
}

}  // namespace proj
}  // namespace bark

#endif  // BARK_PROJ_NORMALIZE_HPP
