// Andrew Naplavkov

#ifndef BARK_DB_CACHER_HPP
#define BARK_DB_CACHER_HPP

#include <algorithm>
#include <atomic>
#include <bark/db/provider.hpp>
#include <bark/detail/lru_cache.hpp>
#include <bark/geometry/geometry.hpp>
#include <bark/proj/bimap.hpp>
#include <boost/functional/hash.hpp>
#include <tuple>

namespace bark::db {

template <class T>
class cacher {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    cacher() : scope_{lru_cache::new_scope()} {}

    std::map<qualified_name, layer_type> cached_dir()
    {
        return std::any_cast<std::map<qualified_name, layer_type>>(
            lru_cache::get_or_invoke(
                scope_, Dir, [&] { return as_mixin().load_dir(); }));
    }

    proj::bimap cached_projection_bimap()
    {
        return std::any_cast<proj::bimap>(
            lru_cache::get_or_invoke(scope_, ProjectionBimap, [&] {
                return as_mixin().load_projection_bimap();
            }));
    }

    table_def cached_table(const qualified_name& tbl_nm)
    {
        return std::any_cast<table_def>(lru_cache::get_or_invoke(
            scope_, tbl_nm, [&] { return as_mixin().load_table(tbl_nm); }));
    }

    geometry::multi_box cached_tiles_first(const qualified_name& lr_nm,
                                           const geometry::box& ext,
                                           const geometry::box& px)
    {
        using namespace boost::geometry;
        auto res = as_mixin().make_tile_coverage(lr_nm, ext, px);
        auto center = return_centroid<geometry::point>(ext);
        std::sort(res.begin(), res.end(), [center](auto& lhs, auto& rhs) {
            return distance(center, return_centroid<geometry::point>(lhs)) <
                   distance(center, return_centroid<geometry::point>(rhs));
        });
        std::stable_partition(res.begin(), res.end(), [&](auto& tile) {
            return lru_cache::contains(scope_, layer_tile{lr_nm, tile});
        });
        return res;
    }

    rowset cached_spatial_objects(const qualified_name& lr_nm,
                                  const geometry::box& ext,
                                  const geometry::box& px)
    {
        return std::any_cast<rowset>(
            lru_cache::get_or_invoke(scope_, layer_tile{lr_nm, ext}, [&] {
                return as_mixin().load_spatial_objects(lr_nm, ext, px);
            }));
    }

    std::string cached_schema()
    {
        return std::any_cast<std::string>(
            lru_cache::get_or_invoke(scope_, CurrentSchema, [&] {
                return as_mixin().load_current_schema();
            }));
    }

    void reset_cache() { scope_ = lru_cache::new_scope(); }

private:
    enum keys { CurrentSchema, Dir, ProjectionBimap };

    struct layer_tile {
        qualified_name name;
        geometry::box extent;

        friend bool operator==(const layer_tile& lhs, const layer_tile& rhs)
        {
            return lhs.tie() == rhs.tie();
        }

        friend size_t hash_value(const layer_tile& that)
        {
            return boost::hash_value(that.tie());
        }

        auto tie() const
        {
            return std::tie(name,
                            extent.min_corner().x(),
                            extent.min_corner().y(),
                            extent.max_corner().x(),
                            extent.max_corner().y());
        }
    };

    std::atomic<lru_cache::scope_type> scope_;
};

}  // namespace bark::db

#endif  // BARK_DB_CACHER_HPP
