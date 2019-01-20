// Andrew Naplavkov

#ifndef BARK_DB_GDAL_DETAIL_LAYER_HPP
#define BARK_DB_GDAL_DETAIL_LAYER_HPP

#include <bark/db/detail/utility.hpp>
#include <bark/db/table_def.hpp>

namespace bark::db::gdal::detail {

class layer {
public:
    layer(GDALDatasetH ds, OGRLayerH lr) : lr_{lr, layer_deleter{ds}} {}
    explicit operator bool() const { return !!lr_; }
    operator OGRLayerH() const { return lr_.get(); }

    table_def table() const
    {
        table_def res;
        res.name = id(OGR_L_GetName(lr_.get()));
        auto feat_def = OGR_L_GetLayerDefn(lr_.get());
        for (int i = 0; i < OGR_FD_GetGeomFieldCount(feat_def); ++i) {
            auto field_def = OGR_FD_GetGeomFieldDefn(feat_def, i);
            column_def col;
            col.name = OGR_GFld_GetNameRef(field_def);
            if (col.name.empty())
                col.name = GeometryColumn;
            col.type = column_type::Geometry;
            col.projection = projection(OGR_GFld_GetSpatialRef(field_def));
            col.tiles = load_tiles();
            res.columns.push_back(col);
            if (OGR_L_TestCapability(lr_.get(), "FastSpatialFilter"))
                res.indexes.push_back({index_type::Secondary, {col.name}});
        }
        for (int i = 0; i < OGR_FD_GetFieldCount(feat_def); ++i) {
            auto field_def = OGR_FD_GetFieldDefn(feat_def, i);
            column_def col;
            col.name = OGR_Fld_GetNameRef(field_def);
            col.type = type(OGR_Fld_GetType(field_def));
            res.columns.push_back(col);
        }
        return res;
    }

private:
    layer_holder lr_;

    static column_type type(OGRFieldType ogr_type)
    {
        switch (ogr_type) {
            case OFTInteger:
            case OFTInteger64:
                return column_type::Integer;
            case OFTReal:
                return column_type::Real;
            case OFTDate:
            case OFTTime:
            case OFTDateTime:
            case OFTString:
                return column_type::Text;
            case OFTBinary:
                return column_type::Blob;
            default:
                throw std::runtime_error("unsupported OGR type: " +
                                         std::to_string(ogr_type));
        }
    }

    rtree load_tiles() const
    {
        auto count = OGR_L_GetFeatureCount(lr_.get(), 1 /*force*/);
        if (!count)
            return rtree{};
        OGREnvelope ext;
        check(OGR_L_GetExtent(lr_.get(), &ext, 1 /*force*/));
        return db::detail::make_tiles(
            count, {{ext.MinX, ext.MinY}, {ext.MaxX, ext.MaxY}});
    }
};

}  // namespace bark::db::gdal::detail

#endif  // BARK_DB_GDAL_DETAIL_LAYER_HPP
