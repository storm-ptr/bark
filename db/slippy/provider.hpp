// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_PROVIDER_HPP
#define BARK_DB_SLIPPY_PROVIDER_HPP

#include <bark/dataset/ostream.hpp>
#include <bark/db/detail/cacher.hpp>
#include <bark/db/provider.hpp>
#include <bark/db/slippy/detail/layers.hpp>
#include <bark/db/slippy/detail/tile.hpp>
#include <bark/detail/curl/jobs.hpp>
#include <bark/geometry/as_binary.hpp>
#include <stdexcept>

namespace bark {
namespace db {
namespace slippy {

class provider : private db::detail::cacher<slippy::provider>,
                 public db::provider {
public:
    friend db::detail::cacher<slippy::provider>;

    layer_to_type_map dir() override { return cached_dir(); }

    std::string projection(const qualified_name&) override
    {
        return detail::projection();
    }

    geometry::box extent(const qualified_name&) override
    {
        auto tf = detail::tile_to_layer_transformer();
        return tf.forward(detail::extent({}));
    }

    double undistorted_scale(const qualified_name& lr_nm,
                             const geometry::view& view) override
    {
        auto tf = detail::tile_to_layer_transformer();
        auto lr = layers_[lr_nm];
        auto px = geometry::pixel(view);
        auto tl = detail::match(tf.backward(px), lr->zmax());
        return geometry::max_scale(tf.forward(detail::pixel(tl)));
    }

    geometry::multi_box tile_coverage(const qualified_name& lr_nm,
                                      const geometry::view& view) override
    {
        return cached_tiles_first(lr_nm, view);
    }

    dataset::rowset spatial_objects(const qualified_name& lr_nm,
                                    const geometry::view& view) override
    {
        return cached_spatial_objects(lr_nm, view);
    }

    command_holder make_command() override
    {
        throw std::logic_error{"not implemented"};
    }

    table_def table(const qualified_name&) override
    {
        throw std::logic_error{"not implemented"};
    }

    table_script script(const table_def&) override
    {
        throw std::logic_error{"not implemented"};
    }

    void page_clause(sql_builder&, size_t, size_t) override
    {
        throw std::logic_error{"not implemented"};
    }

    void refresh() override { reset_cache(); }

private:
    detail::layers layers_;

    layer_to_type_map load_dir() { return layers_.dir(); }

    geometry::multi_box make_tile_coverage(const qualified_name& lr_nm,
                                           const geometry::view& view)
    {
        auto tf = detail::tile_to_layer_transformer();
        auto lr = layers_[lr_nm];
        auto tls = detail::tile_coverage(tf.backward(view), lr->zmax());
        return back_constructor<geometry::multi_box>(
            tls, [&tf](auto& tl) { return tf.forward(detail::extent(tl)); });
    }

    dataset::rowset load_spatial_objects(const qualified_name& lr_nm,
                                         const geometry::view& view)
    {
        auto tf = detail::tile_to_layer_transformer();
        auto lr = layers_[lr_nm];
        auto tls = detail::tile_coverage(tf.backward(view), lr->zmax());

        curl::jobs<detail::tile> jobs;
        for (auto& tl : tls)
            jobs.push(tl, lr->url(tl));

        dataset::ostream os;
        while (!jobs.empty()) {
            auto pair = jobs.pop();
            auto tl = pair.first;
            auto img = pair.second;
            auto ext = tf.forward(detail::extent(tl));
            os << geometry::as_binary(ext) << img << tl.z << tl.x << tl.y
               << lr->url(tl);
        }
        return {{"wkb", "image", "zoom", "x", "y", "url"}, std::move(os).buf()};
    }
};

}  // namespace slippy
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SLIPPY_PROVIDER_HPP
