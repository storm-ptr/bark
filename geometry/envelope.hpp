// Andrew Naplavkov

#ifndef BARK_GEOMETRY_ENVELOPE_HPP
#define BARK_GEOMETRY_ENVELOPE_HPP

#include <bark/geometry/geometry.hpp>

namespace bark::geometry {

template <class T>
auto envelope(const T& geom)
{
    return boost::geometry::return_envelope<box>(geom);
}

box envelope(const geometry&);

inline box envelope(const geometry_collection& coll)
{
    if (coll.empty())
        return {};
    auto res = envelope(coll.front());
    for (auto it = std::next(coll.begin()); it != coll.end(); ++it)
        boost::geometry::expand(res, envelope(*it));
    return res;
}

inline box envelope(const geometry& geom)
{
    return boost::apply_visitor([](const auto& item) { return envelope(item); },
                                geom);
}

inline box envelope(const box_rtree& rtree)
{
    return rtree.empty() ? box{} : envelope(bounds(rtree));
}

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_ENVELOPE_HPP
