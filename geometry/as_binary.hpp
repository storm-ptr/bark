// Andrew Naplavkov

#ifndef BARK_GEOMETRY_AS_BINARY_HPP
#define BARK_GEOMETRY_AS_BINARY_HPP

#include <bark/detail/utility.hpp>
#include <bark/geometry/detail/ostream.hpp>
#include <bark/geometry/geometry.hpp>

namespace bark::geometry {
namespace detail {

struct converter {
    auto operator()(const box& v) const
    {
        polygon res;
        boost::geometry::convert(v, res);
        return res;
    }

    auto operator()(const multi_box& v) const
    {
        return as<multi_polygon>(v, *this);
    }
};

}  // namespace detail

template <class T>
blob as_binary(const T& v)
{
    ostream os;
    os << v;
    return std::move(os.data);
}

inline auto as_binary(const box& v)
{
    return as_binary(detail::converter{}(v));
}

inline auto as_binary(const multi_box& v)
{
    return as_binary(detail::converter{}(v));
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_AS_BINARY_HPP
