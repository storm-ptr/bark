// Andrew Naplavkov

#ifndef BARK_DB_GDAL_DETAIL_DATASET_HPP
#define BARK_DB_GDAL_DETAIL_DATASET_HPP

#include <atomic>
#include <bark/db/gdal/detail/layer.hpp>
#include <bark/db/qualified_name.hpp>
#include <cstdint>
#include <memory>
#include <mutex>

namespace bark {
namespace db {
namespace gdal {
namespace detail {

class dataset {
public:
    explicit dataset(const std::string& file)
    {
        static std::once_flag once_flag;
        std::call_once(once_flag, [] { GDALAllRegister(); });
        ds_.reset(GDALOpenEx(file.c_str(), 0, nullptr, nullptr, nullptr));
        check(!!ds_);
    }

    operator GDALDatasetH() const { return ds_.get(); }

    std::string projection() const
    {
        spatial_reference_holder srs{
            OSRNewSpatialReference(GDALGetProjectionRef(ds_.get()))};
        return detail::projection(srs.get());
    }

    blob_t png() const
    {
        static std::atomic<uint64_t> file_id_{0};

        auto drv = GDALGetDriverByName("PNG");
        check(!!drv);
        auto file = "/vsimem/" + std::to_string(++file_id_) + ".png";
        {
            dataset_holder cp(
                GDALCreateCopy(drv, file.c_str(), ds_.get(), false, 0, 0, 0));
            check(!!cp);
        }
        vsi_l_offset len(0);
        vsi_holder buf(VSIGetMemFileBuffer(file.c_str(), &len, true));
        auto begin = (const uint8_t*)buf.get();
        auto end = begin + len;
        return {begin, end};
    }

    bool has_layers() const { return GDALDatasetGetLayerCount(ds_.get()) > 0; }

    std::vector<qualified_name> layers() const
    {
        std::vector<qualified_name> res;
        int count = GDALDatasetGetLayerCount(ds_.get());
        res.reserve(count);
        for (int i = 0; i < count; ++i) {
            auto lr = GDALDatasetGetLayer(ds_.get(), i);
            std::string col = OGR_L_GetGeometryColumn(lr);
            if (col.empty())
                col = GeometryColumn;
            res.push_back(id(OGR_L_GetName(lr), col));
        }
        return res;
    }

    layer layer_by_name(const qualified_name& tbl) const
    {
        return {nullptr,
                GDALDatasetGetLayerByName(ds_.get(), tbl.back().c_str())};
    }

    layer layer_by_sql(const std::string& sql) const
    {
        return {
            ds_.get(),
            GDALDatasetExecuteSQL(ds_.get(), sql.c_str(), nullptr, "SQLITE")};
    }

private:
    dataset_holder ds_;
};

}  // namespace detail
}  // namespace gdal
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_GDAL_DETAIL_DATASET_HPP
