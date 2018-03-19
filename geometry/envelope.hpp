// Andrew Naplavkov

#ifndef BARK_GEOMETRY_ENVELOPE_HPP
#define BARK_GEOMETRY_ENVELOPE_HPP

#include <bark/geometry/geometry.hpp>
#include <boost/variant/static_visitor.hpp>
#include <stdexcept>

namespace bark {
namespace geometry {

template <typename T>
auto envelope(const T& val)
{
    return boost::geometry::return_envelope<box>(val);
}

inline box envelope(const geometry_collection&);

inline box envelope(const geometry&);

namespace detail {

struct envelope_visitor : boost::static_visitor<box> {
    template <typename T>
    box operator()(const T& val) const
    {
        return envelope(val);
    }

    box operator()(
        const boost::recursive_wrapper<geometry_collection>& val) const
    {
        return envelope(val.get());
    }
};

}  // namespace detail

inline box envelope(const geometry_collection& val)
{
    if (val.empty())
        throw std::runtime_error("envelope error");
    auto res = envelope(val.front());
    for (auto it = std::next(val.begin()); it != val.end(); ++it)
        boost::geometry::expand(res, envelope(*it));
    return res;
}

inline box envelope(const geometry& val)
{
    return boost::apply_visitor(detail::envelope_visitor{}, val);
}

}  // namespace geometry
}  // namespace bark

#endif  // BARK_GEOMETRY_ENVELOPE_HPP
