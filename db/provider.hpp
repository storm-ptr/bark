// Andrew Naplavkov

#ifndef BARK_DB_PROVIDER_HPP
#define BARK_DB_PROVIDER_HPP

#include <bark/db/command.hpp>
#include <bark/db/fwd.hpp>
#include <bark/db/sql_builder.hpp>
#include <bark/db/table_def.hpp>
#include <bark/detail/lru_cache.hpp>
#include <bark/geometry/geometry.hpp>
#include <map>

namespace bark::db {

using layer_to_type_map = std::map<qualified_name, layer_type>;

using busy_exception = bark::lru_cache::busy_exception;

struct table_script {
    qualified_name name;
    std::string sql;
};

/// thread-safe high-level interface
struct provider {
    virtual ~provider() = default;

    virtual layer_to_type_map dir() = 0;

    virtual std::string projection(const qualified_name& layer) = 0;

    virtual geometry::box extent(const qualified_name& layer) = 0;

    virtual double undistorted_scale(const qualified_name& layer,
                                     const geometry::view&) = 0;

    virtual geometry::multi_box tile_coverage(const qualified_name& layer,
                                              const geometry::view&) = 0;

    /// @return {<wkb[, image][, attributes]>}
    virtual rowset spatial_objects(const qualified_name& layer,
                                   const geometry::view&) = 0;

    virtual command_holder make_command() = 0;

    virtual table_def table(const qualified_name& tbl_nm) = 0;

    virtual table_script script(const table_def&) = 0;

    virtual void page_clause(sql_builder&, size_t offset, size_t limit) = 0;

    virtual void refresh() = 0;
};

}  // namespace bark::db

#endif  // BARK_DB_PROVIDER_HPP
