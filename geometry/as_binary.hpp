// Andrew Naplavkov

#ifndef BARK_GEOMETRY_AS_BINARY_HPP
#define BARK_GEOMETRY_AS_BINARY_HPP

#include <bark/geometry/detail/ostream.hpp>
#include <bark/geometry/geometry.hpp>

namespace bark {
namespace geometry {
namespace detail {

inline auto convert(const box& v)
{
    polygon res;
    boost::geometry::convert(v, res);
    return res;
}

inline auto convert(const multi_box& v)
{
    multi_polygon res;
    for (auto& item : v)
        res.push_back(convert(item));
    return res;
}

}  // namespace detail

template <typename T>
blob_t as_binary(const T& v)
{
    detail::ostream os;
    os << v;
    return std::move(os).buf();
}

inline blob_t as_binary(const box& v)
{
    return as_binary(detail::convert(v));
}

inline blob_t as_binary(const multi_box& v)
{
    return as_binary(detail::convert(v));
}

}  // namespace geometry
}  // namespace bark

#endif  // BARK_GEOMETRY_AS_BINARY_HPP
