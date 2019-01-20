// Andrew Naplavkov

#ifndef BARK_GEOMETRY_ENVELOPE_HPP
#define BARK_GEOMETRY_ENVELOPE_HPP

#include <bark/geometry/geometry.hpp>
#include <stdexcept>

namespace bark::geometry {

template <class T>
auto envelope(const T& v)
{
    return boost::geometry::return_envelope<box>(v);
}

inline box envelope(const geometry&);

inline box envelope(const geometry_collection& v)
{
    if (v.empty())
        throw std::runtime_error("invalid envelope");
    auto res = envelope(v.front());
    for (auto it = std::next(v.begin()); it != v.end(); ++it)
        boost::geometry::expand(res, envelope(*it));
    return res;
}

inline box envelope(const geometry& v)
{
    return boost::apply_visitor([](const auto& v) { return envelope(v); }, v);
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_ENVELOPE_HPP
