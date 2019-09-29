// Andrew Naplavkov

#ifndef BARK_DB_GDAL_UTILITY_HPP
#define BARK_DB_GDAL_UTILITY_HPP

#include <bark/proj/normalize.hpp>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>

#include <cpl_conv.h>
#include <cpl_vsi.h>
#include <gdal.h>
#include <ogr_api.h>
#include <ogr_srs_api.h>

namespace bark::db::gdal {

const char GeometryColumn[] = "GEOMETRY";

/// GDALDatasetH
struct dataset_deleter {
    void operator()(void* p) const { GDALClose(p); }
};

using dataset_holder = std::unique_ptr<void, dataset_deleter>;

/// OGRLayerH
class layer_deleter {
    GDALDatasetH ds_;

public:
    explicit layer_deleter(GDALDatasetH ds) : ds_{ds} {}

    void operator()(void* p) const
    {
        if (ds_)
            GDALDatasetReleaseResultSet(ds_, p);
    }
};

using layer_holder = std::unique_ptr<void, layer_deleter>;

/// OGRFeatureH
struct feature_deleter {
    void operator()(void* p) const { OGR_F_Destroy(p); }
};

using feature_holder = std::unique_ptr<void, feature_deleter>;

/// OGRSpatialReferenceH
struct spatial_reference_deleter {
    void operator()(void* p) const { OSRDestroySpatialReference(p); };
};

using spatial_reference_holder =
    std::unique_ptr<void, spatial_reference_deleter>;

struct vsi_deleter {
    void operator()(void* p) const { VSIFree(p); };
};

using vsi_holder = std::unique_ptr<void, vsi_deleter>;

inline std::string error()
{
    std::string res = CPLGetLastErrorMsg();
    if (res.empty())
        res = "GDAL error";
    return res;
}

inline void check(CPLErr res)
{
    if (CE_None != res)
        throw std::runtime_error(error());
}

inline void check(OGRErr res)
{
    if (OGRERR_NONE != res)
        throw std::runtime_error(error());
}

inline void check(bool condition)
{
    if (!condition)
        throw std::runtime_error(error());
}

inline std::string projection(OGRSpatialReferenceH srs)
try {
    check(!!srs);
    char* txt = nullptr;
    auto r = OSRExportToProj4(srs, &txt);
    vsi_holder proj(txt);
    check(r);
    check(!!proj);
    return proj::normalize(txt);
}
catch (const std::exception&) {
    return "";
}

}  // namespace bark::db::gdal

#endif  // BARK_DB_GDAL_UTILITY_HPP
