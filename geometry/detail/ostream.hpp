// Andrew Naplavkov

#ifndef BARK_GEOMETRY_OSTREAM_HPP
#define BARK_GEOMETRY_OSTREAM_HPP

#include <bark/blob.hpp>
#include <bark/detail/wkb.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/map.hpp>

namespace bark::geometry {

class ostream {
public:
    blob data;

    template <class T>
    auto operator<<(const T& r)
    {
        apply_tagged(r);
        return *this;
    }

private:
    void apply(const point& r) { data << double(r.x()) << double(r.y()); }

    void apply(const polygon& r)
    {
        data << uint32_t(1 + r.inners().size());
        apply(r.outer());
        for (auto& inner : r.inners())
            apply(inner);
    }

    void apply(const geometry&) {}

    template <class T>
    void apply(const T& r)
    {
        data << uint32_t(r.size());
        for (auto& item : r) {
            if constexpr (std::is_same_v<T, linestring> ||
                          std::is_same_v<T, polygon::ring_type>)
                apply(item);
            else
                apply_tagged(item);
        }
    }

    void apply_tagged(const geometry& r)
    {
        boost::apply_visitor([&](const auto& r) { apply_tagged(r); }, r);
    }

    template <class T>
    void apply_tagged(const T& r)
    {
        using namespace boost::mpl;
        data << wkb::HostEndian
             << uint32_t(
                    at<map<pair<point, int_<wkb::Point>>,
                           pair<linestring, int_<wkb::Linestring>>,
                           pair<polygon, int_<wkb::Polygon>>,
                           pair<multi_point, int_<wkb::MultiPoint>>,
                           pair<multi_linestring, int_<wkb::MultiLinestring>>,
                           pair<multi_polygon, int_<wkb::MultiPolygon>>,
                           pair<geometry_collection,
                                int_<wkb::GeometryCollection>>>,
                       T>::type::value);
        apply(r);
    }
};

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_OSTREAM_HPP
