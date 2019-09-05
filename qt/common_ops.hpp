// Andrew Naplavkov

#ifndef BARK_QT_COMMON_OPS_HPP
#define BARK_QT_COMMON_OPS_HPP

#include <bark/db/provider.hpp>
#include <bark/qt/common.hpp>
#include <bark/qt/detail/adapt.hpp>
#include <bark/qt/detail/georeference_proj.hpp>
#include <boost/range/adaptor/filtered.hpp>

namespace bark::qt {

inline bool queryable(const link& lnk)
{
    return lnk.queryable;
}

inline auto projection(const layer& lr)
{
    return lr.provider->projection(lr.name);
}

inline auto extent(const layer& lr)
{
    return lr.provider->extent(lr.name);
}

inline auto table(const layer& lr)
{
    return lr.provider->table(qualifier(lr.name));
}

inline auto script(const layer& from, const link& to)
{
    return to.provider->script(table(from));
}

inline auto attr_names(const layer& lr)
{
    return db::names(table(lr).columns | boost::adaptors::filtered(std::not_fn(
                                             same{db::column_type::Geometry})));
}

inline auto tile_coverage(const layer& lr,
                          const geometry::box& ext,
                          const geometry::box& px)
{
    return lr.provider->tile_coverage(lr.name, ext, px);
}

inline auto spatial_objects(const layer& lr,
                            const geometry::box& ext,
                            const geometry::box& px)
{
    return lr.provider->spatial_objects(lr.name, ext, px);
}

inline geometry::box pixel(const georeference& ref)
{
    auto pos = adapt(ref.center);
    return {pos, {pos.x() + ref.scale, pos.y() + ref.scale}};
}

inline geometry::box extent(const georeference& ref)
{
    return backward(ref, QRectF{{}, ref.size});
}

}  // namespace bark::qt

#endif  // BARK_QT_COMMON_OPS_HPP
