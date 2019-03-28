// Andrew Naplavkov

#ifndef BARK_DB_GDAL_BIND_COLUMN_HPP
#define BARK_DB_GDAL_BIND_COLUMN_HPP

#include <bark/db/gdal/detail/utility.hpp>
#include <boost/predef/other/endian.h>
#include <memory>
#include <stdexcept>

namespace bark::db::gdal {

struct column {
    virtual ~column() = default;
    virtual void write(OGRFeatureH feat, int col, variant_ostream& os) = 0;
};

using column_holder = std::unique_ptr<column>;

struct column_int64 : column {
    void write(OGRFeatureH feat, int col, variant_ostream& os) override
    {
        os << (int64_t)OGR_F_GetFieldAsInteger64(feat, col);
    }
};

struct column_double : column {
    void write(OGRFeatureH feat, int col, variant_ostream& os) override
    {
        os << OGR_F_GetFieldAsDouble(feat, col);
    }
};

struct column_string : column {
    void write(OGRFeatureH feat, int col, variant_ostream& os) override
    {
        os << OGR_F_GetFieldAsString(feat, col);
    }
};

struct column_blob : column {
    void write(OGRFeatureH feat, int col, variant_ostream& os) override
    {
        int size(0);
        auto bytes = OGR_F_GetFieldAsBinary(feat, col, &size);
        os << blob_view{(const std::byte*)bytes, (size_t)size};
    }
};

struct column_geom : column {
    blob wkb_;

    void write(OGRFeatureH feat, int col, variant_ostream& os) override
    {
        auto geom = OGR_F_GetGeomFieldRef(feat, col);
        size_t size = geom ? OGR_G_WkbSize(geom) : 0;
        if (size > 0) {
            wkb_.resize(size);
            OGR_G_ExportToWkb(geom,
#if defined BOOST_ENDIAN_LITTLE_BYTE
                              wkbNDR
#elif defined BOOST_ENDIAN_BIG_BYTE
                              wkbXDR
#else
#error byte order
#endif
                              ,
                              (unsigned char*)wkb_.data());
            os << wkb_;
        }
        else
            os << variant_t{};
    }
};

inline column_holder bind_column(OGRFieldType ogr_type)
{
    switch (ogr_type) {
        case OFTInteger:
        case OFTInteger64:
            return std::make_unique<column_int64>();
        case OFTReal:
            return std::make_unique<column_double>();
        case OFTDate:
        case OFTTime:
        case OFTDateTime:
        case OFTString:
            return std::make_unique<column_string>();
        case OFTBinary:
            return std::make_unique<column_blob>();
        default:
            throw std::runtime_error("unsupported OGR type: " +
                                     std::to_string(ogr_type));
    }
}

}  // namespace bark::db::gdal

#endif  // BARK_DB_GDAL_BIND_COLUMN_HPP
