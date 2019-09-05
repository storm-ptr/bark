// Andrew Naplavkov

#ifndef BARK_DB_GDAL_PROVIDER_HPP
#define BARK_DB_GDAL_PROVIDER_HPP

#include <bark/db/detail/cacher.hpp>
#include <bark/db/detail/provider_ops.hpp>
#include <bark/db/detail/utility.hpp>
#include <bark/db/gdal/command.hpp>
#include <bark/db/gdal/detail/dataset.hpp>
#include <bark/db/gdal/detail/georeference.hpp>
#include <bark/geometry/as_binary.hpp>
#include <bark/geometry/envelope.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <iterator>
#include <optional>
#include <stdexcept>

namespace bark::db::gdal {

class provider : private cacher<provider>, public db::provider {
    friend cacher<provider>;

public:
    explicit provider(std::string_view file) : file_{file}
    {
        if (is_raster())
            ref_ = georeference{dataset{file_}};
    }

    std::map<qualified_name, layer_type> dir() override { return cached_dir(); }

    std::string projection(const qualified_name& lr_nm) override
    {
        return is_raster() ? dataset{file_}.projection()
                           : db::column(*this, lr_nm).projection;
    }

    geometry::box extent(const qualified_name& lr_nm) override
    {
        if (is_raster())
            return ref_->extent();
        auto tls = db::column(*this, lr_nm).tiles;
        return tls.empty() ? geometry::box{} : geometry::envelope(bounds(tls));
    }

    geometry::box undistorted_pixel(const qualified_name&,
                                    const geometry::box& px) override
    {
        return is_raster() ? geometry::shift(ref_->pixel(), px) : px;
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
        if (is_raster())
            throw std::logic_error{"not implemented"};
        return command_holder(new command(file_),
                              std::default_delete<db::command>());
    }

    db::table_def table(const qualified_name& tbl_nm) override
    {
        return cached_table(tbl_nm);
    }

    std::pair<qualified_name, std::string> script(const table_def&) override
    {
        throw std::logic_error{"not implemented"};
    }

    void page_clause(sql_builder& bld, size_t offset, size_t limit) override
    {
        limit_page_clause(bld, offset, limit);
    }

    void refresh() override { reset_cache(); }

private:
    const std::string file_;
    std::optional<georeference> ref_;

    std::map<qualified_name, layer_type> load_dir()
    {
        std::map<qualified_name, layer_type> res;
        for (auto& item : dataset{file_}.layers())
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
        auto qry = dataset{file_}.layer_by_sql(bld.sql()).table();
        auto tbl = dataset{file_}.layer_by_name(tbl_nm).table();
        auto diff =
            qry.columns | boost::adaptors::filtered([&](auto& col) {
                return db::find(tbl.columns, col.name) == std::end(tbl.columns);
            });
        auto it = boost::range::find_if(
            tbl.columns, std::not_fn(same{column_type::Geometry}));
        auto count = tbl.columns.size();
        tbl.columns.insert(it, std::begin(diff), std::end(diff));
        if (tbl.columns.size() - count == 1)
            tbl.indexes.insert(tbl.indexes.begin(),
                               {index_type::Primary, names(diff)});
        return tbl;
    }

    geometry::multi_box make_tile_coverage(const qualified_name& lr_nm,
                                           const geometry::box& ext,
                                           const geometry::box&)
    {
        geometry::multi_box res;
        if (is_raster()) {
            auto bbox = this->extent(lr_nm);
            if (boost::geometry::intersects(ext, bbox))
                res.push_back(std::move(bbox));
        }
        else
            db::column(*this, lr_nm)
                .tiles.query(boost::geometry::index::intersects(ext),
                             std::back_inserter(res));
        return res;
    }

    rowset load_spatial_objects(const qualified_name& lr_nm,
                                const geometry::box& ext,
                                const geometry::box&)
    {
        if (is_raster()) {
            variant_ostream os;
            auto bbox = this->extent(lr_nm);
            if (boost::geometry::intersects(ext, bbox))
                os << geometry::as_binary(bbox) << dataset{file_}.png();
            return {{"wkb", "image"}, std::move(os.data)};
        }
        else {
            command cmd(file_);
            cmd.open(lr_nm, ext);
            return fetch_all(cmd);
        }
    }
};

}  // namespace bark::db::gdal

#endif  // BARK_DB_GDAL_PROVIDER_HPP
