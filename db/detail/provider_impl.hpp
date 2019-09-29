// Andrew Naplavkov

#ifndef BARK_DB_PROVIDER_IMPL_HPP
#define BARK_DB_PROVIDER_IMPL_HPP

#include <bark/db/detail/dialect.hpp>
#include <bark/db/detail/pool.hpp>
#include <bark/db/detail/provider_ops.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/geometry/geom_from_wkb.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include <memory>

namespace bark::db {

template <class T>
class provider_impl : public db::provider {
    T& as_mixin() { return static_cast<T&>(*this); }

public:
    provider_impl(std::function<command*()> alloc, dialect_holder dialect)
        : pool_{std::make_shared<pool>(std::move(alloc))}
        , dialect_{std::move(dialect)}
    {
    }

    std::map<qualified_name, layer_type> dir() override
    {
        return as_mixin().cached_dir();
    }

    std::string projection(const qualified_name& lr_nm) override
    {
        return column(*this, lr_nm).projection;
    }

    geometry::box extent(const qualified_name& lr_nm) override
    {
        return db::extent(column(*this, lr_nm));
    }

    geometry::box undistorted_pixel(const qualified_name&,
                                    const geometry::box& px) override
    {
        return px;
    }

    geometry::multi_box tile_coverage(const qualified_name& lr_nm,
                                      const geometry::box& ext,
                                      const geometry::box& px) override
    {
        return as_mixin().cached_tiles_first(lr_nm, ext, px);
    }

    rowset spatial_objects(const qualified_name& lr_nm,
                           const geometry::box& ext,
                           const geometry::box& px) override
    {
        return as_mixin().cached_spatial_objects(lr_nm, ext, px);
    }

    command_holder make_command() override { return pool_->make_command(); }

    table_def table(const qualified_name& tbl_nm) override
    {
        return as_mixin().cached_table(tbl_nm);
    }

    std::pair<qualified_name, std::string> script(const table_def& tbl) override
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

    std::map<qualified_name, layer_type> load_dir()
    try {
        enum columns { Schema, Table, Column };
        auto res = std::map<qualified_name, layer_type>{};
        auto bld = builder(as_mixin());
        as_dialect().geometries_sql(bld);
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : select(rows))
            res.insert({id(boost::lexical_cast<std::string>(row[Schema]),
                           boost::lexical_cast<std::string>(row[Table]),
                           boost::lexical_cast<std::string>(row[Column])),
                        layer_type::Geometry});
        return res;
    }
    catch (const std::exception&) {
        return {};
    }

    geometry::multi_box make_tile_coverage(const qualified_name& lr_nm,
                                           const geometry::box& ext,
                                           const geometry::box&)
    {
        geometry::multi_box res;
        column(*this, lr_nm)
            .tiles.query(boost::geometry::index::intersects(ext),
                         std::back_inserter(res));
        return res;
    }

    rowset load_spatial_objects(const qualified_name& lr_nm,
                                const geometry::box& ext,
                                const geometry::box&)
    {
        auto tbl = table(qualifier(lr_nm));
        auto col_nm = lr_nm.back();
        auto it = db::find(tbl.columns, col_nm);
        std::rotate(tbl.columns.begin(), it, std::next(it));
        auto bld = builder(*this);
        bld << "SELECT " << list{tbl.columns, ", ", decode} << " FROM "
            << tbl.name << " WHERE ";
        as_dialect().window_clause(bld, tbl, col_nm, ext);
        return fetch_all(*this, bld);
    }

    std::string load_current_schema()
    {
        auto bld = builder(as_mixin());
        as_dialect().current_schema_sql(bld);
        return fetch_or_default<std::string>(as_mixin(), bld);
    }

    rtree load_tiles(const qualified_name& col_nm, std::string_view type)
    {
        auto bld = builder(as_mixin());
        as_dialect().extent_sql(bld, col_nm, type);
        auto rows = fetch_all(as_mixin(), bld);
        auto row = select(rows).front();
        auto count = boost::lexical_cast<size_t>(row[0]);
        auto ext = geometry::box{};
        if (count) {
            if (row.size() > 2)
                ext = {{boost::lexical_cast<double>(row[1]),
                        boost::lexical_cast<double>(row[2])},
                       {boost::lexical_cast<double>(row[3]),
                        boost::lexical_cast<double>(row[4])}};
            else
                ext = geometry::envelope(
                    geometry::poly_from_wkb(std::get<blob_view>(row[1])));
        }
        return make_tiles(count, ext);
    }

    void prepare_geometry_column(const qualified_name& tbl_nm,
                                 column_def& col,
                                 std::string_view type)
    {
        auto col_nm = id(tbl_nm, col.name);
        auto srid = as_mixin().load_projection(col_nm, type);
        col.projection = as_mixin().find_proj(srid);
        col.decoder = as_dialect().geometry_decoder();
        col.encoder = as_dialect().geometry_encoder(type, srid);
        col.tiles = as_mixin().load_tiles(col_nm, type);
    }

private:
    std::shared_ptr<pool> pool_;
    dialect_holder dialect_;
};

}  // namespace bark::db

#endif  // BARK_DB_PROVIDER_IMPL_HPP
