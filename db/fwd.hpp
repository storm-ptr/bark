// Andrew Naplavkov

#ifndef BARK_DB_FWD_HPP
#define BARK_DB_FWD_HPP

#include <memory>

namespace bark::db {

enum class layer_type { Invalid, Geometry, Raster };

struct provider;

using provider_ptr = std::shared_ptr<provider>;

}  // namespace bark::db

#endif  // BARK_DB_FWD_HPP
