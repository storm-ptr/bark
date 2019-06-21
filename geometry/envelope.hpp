// Andrew Naplavkov

#ifndef BARK_GEOMETRY_ENVELOPE_HPP
#define BARK_GEOMETRY_ENVELOPE_HPP

#include <bark/geometry/geometry.hpp>
#include <stdexcept>

namespace bark::geometry {

template <class T>
auto envelope(const T& val)
{
    return boost::geometry::return_envelope<box>(val);
}

box envelope(const geometry&);

inline box envelope(const geometry_collection& val)
{
    if (val.empty())
        throw std::runtime_error("undefined envelope");
    auto res = envelope(val.front());
    for (auto it = std::next(val.begin()); it != val.end(); ++it)
        boost::geometry::expand(res, envelope(*it));
    return res;
}

inline box envelope(const geometry& val)
{
    return boost::apply_visitor([](const auto& v) { return envelope(v); }, val);
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_ENVELOPE_HPP
