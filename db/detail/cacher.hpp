// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_CACHER_HPP
#define BARK_DB_DETAIL_CACHER_HPP

#include <algorithm>
#include <atomic>
#include <bark/db/provider.hpp>
#include <bark/db/qualified_name.hpp>
#include <bark/detail/lru_cache.hpp>
#include <bark/geometry/geometry.hpp>
#include <bark/proj/bimap.hpp>
#include <boost/functional/hash.hpp>
#include <boost/operators.hpp>
#include <tuple>

namespace bark {
namespace db {
namespace detail {

template <typename T>
class cacher {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    cacher() : scope_{lru_cache::next_scope()} {}

    layer_to_type_map cached_dir()
    {
        return boost::any_cast<layer_to_type_map>(lru_cache::get_or_load(
            scope_, Registry, [&] { return as_mixin().load_dir(); }));
    }

    proj::bimap cached_projection_bimap()
    {
        return boost::any_cast<proj::bimap>(
            lru_cache::get_or_load(scope_, ProjectionBimap, [&] {
                return as_mixin().load_projection_bimap();
            }));
    }

    table_def cached_table(const qualified_name& tbl_nm)
    {
        return boost::any_cast<table_def>(lru_cache::get_or_load(
            scope_, tbl_nm, [&] { return as_mixin().load_table(tbl_nm); }));
    }

    geometry::multi_box cached_tiles_first(const qualified_name& lr_nm,
                                           const geometry::view& view)
    {
        using namespace boost::geometry;
        auto res = as_mixin().make_tile_coverage(lr_nm, view);
        auto center = return_centroid<geometry::point>(view.extent);
        std::sort(res.begin(), res.end(), [center](auto& lhs, auto& rhs) {
            return distance(center, return_centroid<geometry::point>(lhs)) <
                   distance(center, return_centroid<geometry::point>(rhs));
        });
        std::stable_partition(res.begin(), res.end(), [&](auto& tile) {
            return lru_cache::contains(scope_, layer_tile{lr_nm, tile});
        });
        return res;
    }

    dataset::rowset cached_spatial_objects(const qualified_name& lr_nm,
                                           const geometry::view& view)
    {
        return boost::any_cast<dataset::rowset>(
            lru_cache::get_or_load(scope_, layer_tile{lr_nm, view.extent}, [&] {
                return as_mixin().load_spatial_objects(lr_nm, view);
            }));
    }

    std::string cached_schema()
    {
        return boost::any_cast<std::string>(lru_cache::get_or_load(
            scope_, Schema, [&] { return as_mixin().load_current_schema(); }));
    }

    void reset_cache() { scope_ = lru_cache::next_scope(); }

private:
    enum keys { ProjectionBimap, Registry, Schema };

    class layer_tile : boost::equality_comparable<layer_tile> {
    public:
        layer_tile(const qualified_name& layer, const geometry::box& extent)
            : name_(layer), extent_(extent)
        {
        }

        friend bool operator==(const layer_tile& lhs, const layer_tile& rhs)
        {
            return lhs.tie() == rhs.tie();
        };

        friend size_t hash_value(const layer_tile& that)
        {
            return boost::hash_value(that.tie());
        };

    private:
        const qualified_name name_;
        const geometry::box extent_;

        auto tie() const
        {
            return std::tie(name_,
                            extent_.min_corner().x(),
                            extent_.min_corner().y(),
                            extent_.max_corner().x(),
                            extent_.max_corner().y());
        }
    };

    std::atomic<lru_cache::scope_type> scope_;
};

}  // namespace detail
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_DETAIL_CACHER_HPP
