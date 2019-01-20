// Andrew Naplavkov

#ifndef BARK_DB_GDAL_PROVIDER_HPP
#define BARK_DB_GDAL_PROVIDER_HPP

#include <bark/db/detail/cacher.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/db/gdal/command.hpp>
#include <bark/db/gdal/detail/dataset.hpp>
#include <bark/db/gdal/detail/frame.hpp>
#include <bark/db/provider_ops.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/envelope.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <iterator>
#include <optional>
#include <stdexcept>

namespace bark::db::gdal {

class provider : private db::detail::cacher<gdal::provider>,
                 public db::provider {
    friend db::detail::cacher<gdal::provider>;

public:
    explicit provider(std::string_view file) : file_{file}
    {
        if (is_raster())
            frame_ = detail::frame{detail::dataset{file_}};
    }

    layer_to_type_map dir() override { return cached_dir(); }

    std::string projection(const qualified_name& lr_nm) override
    {
        return is_raster() ? detail::dataset{file_}.projection()
                           : column(*this, lr_nm).projection;
    }

    geometry::box extent(const qualified_name& lr_nm) override
    {
        if (is_raster())
            return frame_->extent();
        auto tls = column(*this, lr_nm).tiles;
        return tls.empty() ? geometry::box{} : geometry::envelope(bounds(tls));
    }

    double undistorted_scale(const qualified_name&,
                             const geometry::view& view) override
    {
        return is_raster() ? geometry::max_scale(frame_->pixel()) : view.scale;
    }

    geometry::multi_box tile_coverage(const qualified_name& lr_nm,
                                      const geometry::view& view) override
    {
        return cached_tiles_first(lr_nm, view);
    }

    rowset spatial_objects(const qualified_name& lr_nm,
                           const geometry::view& view) override
    {
        return cached_spatial_objects(lr_nm, view);
    }

    command_holder make_command() override
    {
        if (is_raster())
            throw std::logic_error{"not implemented"};
        return command_holder(new gdal::command(file_),
                              [](db::command* cmd) { delete cmd; });
    }

    db::table_def table(const qualified_name& tbl_nm) override
    {
        return cached_table(tbl_nm);
    }

    table_script script(const table_def&) override
    {
        throw std::logic_error{"not implemented"};
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        db::detail::limit_page_clause(bld, offset, limit);
    }

    void refresh() override { reset_cache(); }

private:
    const std::string file_;
    std::optional<detail::frame> frame_;

    layer_to_type_map load_dir()
    {
        layer_to_type_map res;
        for (auto& item : detail::dataset{file_}.layers())
            res.emplace(item, layer_type::Geometry);
        if (res.empty())
            res.emplace(id(file_), layer_type::Raster);
        return res;
    }

    bool is_raster()
    {
        auto reg = dir();
        return reg.size() == 1 && reg.begin()->second == layer_type::Raster;
    }

    auto load_table(const qualified_name& tbl_nm)
    {
        auto bld = builder(*this);
        bld << "SELECT * FROM " << tbl_nm << " LIMIT 0";
        auto qry = detail::dataset{file_}.layer_by_sql(bld.sql()).table();
        auto tbl = detail::dataset{file_}.layer_by_name(tbl_nm).table();
        auto diff = set_difference(qry.columns, tbl.columns);
        auto it = boost::range::find_if(tbl.columns, is_not_geom);
        tbl.columns.insert(it, diff.begin(), diff.end());
        if (diff.size() == 1)
            tbl.indexes.insert(tbl.indexes.begin(),
                               {index_type::Primary, column_names(diff)});
        return tbl;
    }

    geometry::multi_box make_tile_coverage(const qualified_name& lr_nm,
                                           const geometry::view& view)
    {
        geometry::multi_box res;
        if (is_raster()) {
            auto bbox = this->extent(lr_nm);
            if (boost::geometry::intersects(view.extent, bbox))
                res.push_back(std::move(bbox));
        }
        else
            column(*this, lr_nm)
                .tiles.query(boost::geometry::index::intersects(view.extent),
                             std::back_inserter(res));
        return res;
    }

    rowset load_spatial_objects(const qualified_name& lr_nm,
                                const geometry::view& view)
    {
        if (is_raster()) {
            variant_ostream os;
            auto bbox = this->extent(lr_nm);
            if (boost::geometry::intersects(view.extent, bbox))
                os << geometry::as_binary(bbox) << detail::dataset{file_}.png();
            return {{"wkb", "image"}, std::move(os.data)};
        }
        else {
            gdal::command cmd(file_);
            cmd.open(lr_nm, view.extent);
            return fetch_all(cmd);
        }
    }
};

}  // namespace bark::db::gdal

#endif  // BARK_DB_GDAL_PROVIDER_HPP
