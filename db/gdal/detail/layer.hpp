// Andrew Naplavkov

#ifndef BARK_DB_GDAL_LAYER_HPP
#define BARK_DB_GDAL_LAYER_HPP

#include <bark/db/detail/utility.hpp>
#include <bark/db/meta.hpp>

namespace bark::db::gdal {

class layer {
public:
    layer(GDALDatasetH ds, OGRLayerH lr) : lr_{lr, layer_deleter{ds}} {}
    explicit operator bool() const { return !!lr_; }
    operator OGRLayerH() const { return lr_.get(); }

    meta::table table() const
    {
        meta::table res;
        res.name = id(OGR_L_GetName(lr_.get()));
        auto feat_def = OGR_L_GetLayerDefn(lr_.get());
        for (int i = 0; i < OGR_FD_GetGeomFieldCount(feat_def); ++i) {
            auto field_def = OGR_FD_GetGeomFieldDefn(feat_def, i);
            meta::column col;
            col.name = OGR_GFld_GetNameRef(field_def);
            if (col.name.empty())
                col.name = GeometryColumn;
            col.type = meta::column_type::Geometry;
            col.projection = projection(OGR_GFld_GetSpatialRef(field_def));
            col.tiles = load_tiles();
            res.columns.push_back(col);
            if (OGR_L_TestCapability(lr_.get(), "FastSpatialFilter"))
                res.indexes.push_back(
                    {meta::index_type::Secondary, {col.name}});
        }
        for (int i = 0; i < OGR_FD_GetFieldCount(feat_def); ++i) {
            auto field_def = OGR_FD_GetFieldDefn(feat_def, i);
            meta::column col;
            col.name = OGR_Fld_GetNameRef(field_def);
            col.type = type(OGR_Fld_GetType(field_def));
            res.columns.push_back(col);
        }
        return res;
    }

private:
    layer_holder lr_;

    static meta::column_type type(OGRFieldType ogr_type)
    {
        switch (ogr_type) {
            case OFTInteger:
            case OFTInteger64:
                return meta::column_type::Integer;
            case OFTReal:
                return meta::column_type::Real;
            case OFTDate:
            case OFTTime:
            case OFTDateTime:
            case OFTString:
                return meta::column_type::Text;
            case OFTBinary:
                return meta::column_type::Blob;
            default:
                throw std::runtime_error("unsupported OGR type: " +
                                         std::to_string(ogr_type));
        }
    }

    geometry::box_rtree load_tiles() const
    {
        auto count = OGR_L_GetFeatureCount(lr_.get(), 1 /*force*/);
        if (!count)
            return geometry::box_rtree{};
        OGREnvelope ext;
        check(OGR_L_GetExtent(lr_.get(), &ext, 1 /*force*/));
        return make_tiles(count, {{ext.MinX, ext.MinY}, {ext.MaxX, ext.MaxY}});
    }
};

}  // namespace bark::db::gdal

#endif  // BARK_DB_GDAL_LAYER_HPP
