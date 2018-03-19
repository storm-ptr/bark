// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_PROVIDER_IMPL_HPP
#define BARK_DB_DETAIL_PROVIDER_IMPL_HPP

#include <bark/db/detail/common.hpp>
#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/pool.hpp>
#include <bark/db/provider.hpp>
#include <bark/geometry/geom_from_wkb.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include <memory>

namespace bark {
namespace db {
namespace detail {

template <typename T>
class provider_impl : public db::provider {
    T& as_mixin() { return static_cast<T&>(*this); }

public:
    provider_impl(command_allocator alloc, dialect_holder dialect)
        : pool_{std::make_shared<pool>(std::move(alloc))}
        , dialect_{std::move(dialect)}
    {
    }

    layer_to_type_map dir() override { return as_mixin().cached_dir(); }

    std::string projection(const qualified_name& lr_nm) override
    {
        return column(*this, lr_nm).projection;
    }

    geometry::box extent(const qualified_name& lr_nm) override
    {
        return db::extent(column(*this, lr_nm));
    }

    double undistorted_scale(const qualified_name&,
                             const geometry::view& view) override
    {
        return view.scale;
    }

    geometry::multi_box tile_coverage(const qualified_name& lr_nm,
                                      const geometry::view& view) override
    {
        return as_mixin().cached_tiles_first(lr_nm, view);
    }

    dataset::rowset spatial_objects(const qualified_name& lr_nm,
                                    const geometry::view& view) override
    {
        return as_mixin().cached_spatial_objects(lr_nm, view);
    }

    command_holder make_command() override { return pool_->make_command(); }

    table_def table(const qualified_name& tbl_nm) override
    {
        return as_mixin().cached_table(tbl_nm);
    }

    table_script script(const table_def& tbl) override
    {
        return as_mixin().make_script(tbl);
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        as_dialect().page_clause(bld, offset, limit);
    }

    void refresh() override { as_mixin().reset_cache(); }

protected:
    dialect& as_dialect() { return *dialect_.get(); }

    layer_to_type_map load_dir() try {
        layer_to_type_map res;
        auto bld = builder(as_mixin());
        as_dialect().geometries_sql(bld);
        for (auto& row : fetch_all(as_mixin(), bld))
            res.insert({id(boost::lexical_cast<std::string>(row[0]),
                           boost::lexical_cast<std::string>(row[1]),
                           boost::lexical_cast<std::string>(row[2])),
                        layer_type::Geometry});
        return res;
    }
    catch (const std::exception&) {
        return {};
    }

    geometry::multi_box make_tile_coverage(const qualified_name& lr_nm,
                                           const geometry::view& view)
    {
        geometry::multi_box res;
        column(*this, lr_nm)
            .tiles.query(boost::geometry::index::intersects(view.extent),
                         std::back_inserter(res));
        return res;
    }

    dataset::rowset load_spatial_objects(const qualified_name& lr_nm,
                                         const geometry::view& view)
    {
        auto tbl = table(qualifier(lr_nm));
        auto col_nm = lr_nm.back();
        auto it = find(tbl.columns, col_nm);
        std::rotate(tbl.columns.begin(), it, std::next(it));
        auto bld = builder(*this);
        bld << "SELECT " << list(tbl.columns, ", ", decode) << " FROM "
            << tbl.name << " WHERE ";
        as_dialect().window_clause(bld, tbl, col_nm, view.extent);
        return fetch_all(*this, bld);
    }

    std::string load_current_schema()
    {
        auto bld = builder(as_mixin());
        as_dialect().current_schema_sql(bld);
        return fetch_or_default<std::string>(as_mixin(), bld);
    }

    rtree load_tiles(const qualified_name& col_nm,
                     boost::string_view type_lcase)
    {
        auto bld = builder(as_mixin());
        as_dialect().extent_sql(bld, col_nm, type_lcase);
        auto rows = fetch_all(as_mixin(), bld);
        auto row = *rows.begin();
        auto count = boost::lexical_cast<size_t>(row[0]);
        geometry::box ext{};
        if (count) {
            if (row.size() > 2)
                ext = {{boost::lexical_cast<double>(row[1]),
                        boost::lexical_cast<double>(row[2])},
                       {boost::lexical_cast<double>(row[3]),
                        boost::lexical_cast<double>(row[4])}};
            else
                ext = geometry::envelope(geometry::poly_from_wkb(
                    boost::get<blob_view>(row[1]).data()));
        }
        return make_tiles(count, ext);
    }

    void prepare_geometry_column(const qualified_name& tbl_nm,
                                 column_def& col,
                                 boost::string_view type_lcase)
    {
        auto col_nm = id(tbl_nm, col.name);
        auto srid = as_mixin().load_projection(col_nm, type_lcase);
        col.projection = as_mixin().find_proj(srid);
        col.decoder = as_dialect().geometry_decoder();
        col.encoder = as_dialect().geometry_encoder(type_lcase, srid);
        col.tiles = as_mixin().load_tiles(col_nm, type_lcase);
    }

private:
    std::shared_ptr<pool> pool_;
    dialect_holder dialect_;
};

}  // namespace detail
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_DETAIL_PROVIDER_IMPL_HPP
