// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_PROVIDER_HPP
#define BARK_DB_SLIPPY_PROVIDER_HPP

#include <bark/db/detail/cacher.hpp>
#include <bark/db/provider.hpp>
#include <bark/db/slippy/detail/layers.hpp>
#include <bark/db/slippy/detail/tile.hpp>
#include <bark/detail/curl.hpp>
#include <bark/geometry/as_binary.hpp>
#include <stdexcept>

namespace bark::db::slippy {

class provider : private cacher<provider>, public db::provider {
public:
    friend cacher<provider>;

    layer_to_type_map dir() override { return cached_dir(); }

    std::string projection(const qualified_name&) override
    {
        return slippy::projection();
    }

    geometry::box extent(const qualified_name&) override
    {
        auto tf = tile_to_layer_transformer();
        return tf.forward(slippy::extent({}));
    }

    geometry::box undistorted_pixel(const qualified_name& lr_nm,
                                    const geometry::box& px) override
    {
        auto tf = tile_to_layer_transformer();
        auto lr = layers_[lr_nm];
        auto tl = match(tf.backward(px), lr->zmax());
        return geometry::move_to(tf.forward(slippy::pixel(tl)), px);
    }

    geometry::multi_box tile_coverage(const qualified_name& lr_nm,
                                      const geometry::box& ext,
                                      const geometry::box& px) override
    {
        return cached_tiles_first(lr_nm, ext, px);
    }

    rowset spatial_objects(const qualified_name& lr_nm,
                           const geometry::box& ext,
                           const geometry::box& px) override
    {
        return cached_spatial_objects(lr_nm, ext, px);
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
    layers layers_;

    layer_to_type_map load_dir() { return layers_.dir(); }

    geometry::multi_box make_tile_coverage(const qualified_name& lr_nm,
                                           const geometry::box& ext,
                                           const geometry::box& px)
    {
        auto tf = tile_to_layer_transformer();
        auto lr = layers_[lr_nm];
        auto z = match(tf.backward(px), lr->zmax()).z;
        auto tls = slippy::tile_coverage(tf.backward(ext), z);
        auto op = [&tf](auto& tl) { return tf.forward(slippy::extent(tl)); };
        return as<geometry::multi_box>(tls, op);
    }

    rowset load_spatial_objects(const qualified_name& lr_nm,
                                const geometry::box& ext,
                                const geometry::box& px)
    {
        auto tf = tile_to_layer_transformer();
        auto lr = layers_[lr_nm];
        auto z = match(tf.backward(px), lr->zmax()).z;
        auto tls = slippy::tile_coverage(tf.backward(ext), z);

        bark::curl<tile> jobs;
        for (auto& tl : tls)
            jobs.push(tl, lr->url(tl));

        variant_ostream os;
        while (!jobs.empty()) {
            auto [tl, img] = jobs.pop();
            os << geometry::as_binary(tf.forward(slippy::extent(tl))) << *img
               << tl.z << tl.x << tl.y << lr->url(tl);
        }
        return {{"wkb", "image", "zoom", "x", "y", "url"}, std::move(os.data)};
    }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_PROVIDER_HPP
