// Andrew Naplavkov

#ifndef BARK_GEOMETRY_AS_BINARY_HPP
#define BARK_GEOMETRY_AS_BINARY_HPP

#include <bark/detail/utility.hpp>
#include <bark/geometry/detail/ostream.hpp>
#include <bark/geometry/geometry.hpp>

namespace bark::geometry {
namespace detail {

struct converter {
    auto operator()(const box& val) const
    {
        polygon res;
        boost::geometry::convert(val, res);
        return res;
    }

    auto operator()(const multi_box& val) const
    {
        return as<multi_polygon>(val, *this);
    }
};

}  // namespace detail

template <class T>
blob as_binary(const T& val)
{
    ostream os;
    os << val;
    return std::move(os.data);
}

inline auto as_binary(const box& val)
{
    return as_binary(detail::converter{}(val));
}

inline auto as_binary(const multi_box& val)
{
    return as_binary(detail::converter{}(val));
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_AS_BINARY_HPP
