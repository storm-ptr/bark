// Andrew Naplavkov

#ifndef BARK_DB_GDAL_COMMAND_HPP
#define BARK_DB_GDAL_COMMAND_HPP

#include <bark/db/command.hpp>
#include <bark/db/detail/meta_ops.hpp>
#include <bark/db/detail/transaction.hpp>
#include <bark/db/gdal/detail/bind_column.hpp>
#include <bark/db/gdal/detail/dataset.hpp>
#include <bark/db/gdal/detail/layer.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <iterator>
#include <mutex>
#include <stdexcept>

namespace bark::db::gdal {

class command : public db::command, private transaction<gdal::command> {
    friend transaction<gdal::command>;

public:
    explicit command(const std::string& file) : ds_{file}, lr_(nullptr, nullptr)
    {
    }

    sql_quoted_identifier quoted_identifier() override
    {
        return [](auto id) { return concat('"', id, '"'); };
    }

    sql_parameter_marker parameter_marker() override { return nullptr; }

    void exec(const sql_builder& bld) override
    {
        if (!bld.params().empty())
            throw std::logic_error{"not implemented"};
        reset_lr(ds_.layer_by_sql(bld.sql()));
    }

    std::vector<std::string> columns() override
    {
        reset_cols();
        auto feat_def = OGR_L_GetLayerDefn(lr_);
        for (int i = 0; i < OGR_FD_GetGeomFieldCount(feat_def); ++i)
            geoms_.push_back(std::make_unique<column_geom>());
        for (int i = 0; i < OGR_FD_GetFieldCount(feat_def); ++i) {
            auto field_def = OGR_FD_GetFieldDefn(feat_def, i);
            auto ogr_type = OGR_Fld_GetType(field_def);
            cols_.push_back(bind_column(ogr_type));
        }
        return names(lr_.table().columns);
    }

    bool fetch(variant_ostream& os) override
    {
        if (geoms_.empty() && cols_.empty())
            columns();
        if (geoms_.empty() && cols_.empty())
            return false;
        feature_holder feat{OGR_L_GetNextFeature(lr_)};
        if (!feat)
            return false;
        for (size_t i = 0; i < geoms_.size(); ++i)
            geoms_[i]->write(feat.get(), (int)i, os);
        for (size_t i = 0; i < cols_.size(); ++i) {
            if (OGR_F_IsFieldSet(feat.get(), (int)i))
                cols_[i]->write(feat.get(), (int)i, os);
            else
                os << variant_t{};
        }
        return true;
    }

    void set_autocommit(bool autocommit) override
    {
        transaction::set_autocommit(autocommit);
    }

    void commit() override { transaction::commit(); }

    void open(const qualified_name& layer, const geometry::box& bbox)
    {
        reset_lr(ds_.layer_by_name(qualifier(layer)));
        OGR_L_SetSpatialFilterRect(lr_,
                                   geometry::left(bbox),
                                   geometry::bottom(bbox),
                                   geometry::right(bbox),
                                   geometry::top(bbox));
    }

private:
    dataset ds_;
    layer lr_;
    std::vector<column_holder> geoms_;
    std::vector<column_holder> cols_;

    void reset_cols()
    {
        geoms_.clear();
        cols_.clear();
    }

    void reset_lr(layer lr)
    {
        reset_cols();
        lr_ = std::move(lr);
        check(!!lr_);
    }
};

}  // namespace bark::db::gdal

#endif  // BARK_DB_GDAL_COMMAND_HPP
