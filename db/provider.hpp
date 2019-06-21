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

enum class layer_type { Invalid, Geometry, Raster };

using busy_exception = lru_cache::busy_exception;

/// Thread-safe interface for spatial data source
struct provider {
    virtual ~provider() = default;

    /// Returns layers in this data source
    virtual std::map<qualified_name, layer_type> dir() = 0;

    /// Returns PROJ.4 string for the spatial reference system
    virtual std::string projection(const qualified_name& layer) = 0;

    /// Returns the minimum bounding rectangle of the data set
    virtual geometry::box extent(const qualified_name& layer) = 0;

    /// Changes the pixel to undistort the raster
    virtual geometry::box undistorted_pixel(const qualified_name& layer,
                                            const geometry::box& pixel) = 0;

    /// Returns a balanced data grid.

    /// @param layer is a spatial data set;
    /// @param extent is a spatial filter;
    /// @param pixel selects the level of the raster pyramid.
    virtual geometry::multi_box tile_coverage(const qualified_name& layer,
                                              const geometry::box& extent,
                                              const geometry::box& pixel) = 0;

    /// Returns @ref rowset with spatial data set.

    /// Columns @code GEOMETRY[,IMAGE][,ATTRIBUTES...] @endcode
    /// @param layer is a spatial data set;
    /// @param extent is a spatial filter;
    /// @param pixel selects the level of the raster pyramid.
    virtual rowset spatial_objects(const qualified_name& layer,
                                   const geometry::box& extent,
                                   const geometry::box& pixel) = 0;

    /// Returns SQL command interface
    virtual command_holder make_command() = 0;

    /// Describes table
    virtual table_def table(const qualified_name& tbl_nm) = 0;

    /// Returns name and DDL command to create a table
    virtual std::pair<qualified_name, std::string> script(const table_def&) = 0;

    /// Outputs SQL LIMIT clause
    virtual void page_clause(sql_builder&, size_t offset, size_t limit) = 0;

    /// Resets LRU cache
    virtual void refresh() = 0;
};

}  // namespace bark::db

#endif  // BARK_DB_PROVIDER_HPP
