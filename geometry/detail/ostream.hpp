// Andrew Naplavkov

#ifndef BARK_GEOMETRY_DETAIL_WKB_OSTREAM_HPP
#define BARK_GEOMETRY_DETAIL_WKB_OSTREAM_HPP

#include <bark/detail/blob_stream.hpp>
#include <bark/detail/wkb/common.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/map.hpp>
#include <boost/variant/static_visitor.hpp>

namespace bark {
namespace geometry {
namespace detail {

class ostream {
public:
    blob_view buf() const& { return os_.buf(); }
    blob_t buf() && { return std::move(os_).buf(); };

    template <typename T>
    auto operator<<(const T& r)
    {
        apply_tagged(r);
        return *this;
    }

private:
    bark::detail::blob_ostream os_;

    void apply(const point& r) { os_ << double(r.x()) << double(r.y()); }

    void apply(const polygon& r)
    {
        os_ << uint32_t(1 + r.inners().size());
        apply(r.outer());
        for (auto& inner : r.inners())
            apply(inner);
    }

    void apply(const geometry&) {}

    template <typename T>
    void apply(const T& r)
    {
        os_ << uint32_t(r.size());
        for (auto& item : r) {
            if (is_same<T, linestring, polygon::ring_type>())
                apply(item);
            else
                apply_tagged(item);
        }
    }

    class visitor : public boost::static_visitor<void> {
        ostream& os_;

    public:
        explicit visitor(ostream& os) : os_(os) {}

        template <typename T>
        void operator()(const T& r) const
        {
            os_.apply_tagged(r);
        }

        void operator()(
            const boost::recursive_wrapper<geometry_collection>& r) const
        {
            os_.apply_tagged(r.get());
        }
    };

    void apply_tagged(const geometry& r)
    {
        boost::apply_visitor(visitor{*this}, r);
    }

    template <typename T>
    void apply_tagged(const T& r)
    {
        using namespace boost::mpl;
        os_ << wkb::HostEndian
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

}  // namespace detail
}  // namespace geometry
}  // namespace bark

#endif  // BARK_GEOMETRY_DETAIL_WKB_OSTREAM_HPP
