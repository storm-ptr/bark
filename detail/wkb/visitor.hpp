// Andrew Naplavkov

/// @see http://edndoc.esri.com/arcsde/9.1/general_topics/wkb_representation.htm

#ifndef BARK_DETAIL_WKB_VISITOR_HPP
#define BARK_DETAIL_WKB_VISITOR_HPP

#include <bark/detail/wkb/istream.hpp>
#include <cstdint>
#include <stdexcept>

namespace bark {
namespace detail {
namespace wkb {

struct vertex {
    template <typename Visitor>
    static auto accept(istream& is, Visitor& vi)
    {
        auto x = is.read_double();
        auto y = is.read_double();
        return vi(x, y);
    }
};

template <typename T>
struct chain {
    template <typename Visitor>
    static auto accept(istream& is, Visitor& vi)
    {
        auto count = is.read_uint32();
        auto sum = vi(count, chain{});
        for (decltype(count) i = 0; i < count; ++i)
            vi(sum, T::accept(is, vi), chain{});
        return sum;
    }
};

template <uint32_t Code, typename T>
struct tagged {
    template <typename Visitor>
    static auto accept(istream& is, Visitor& vi)
    {
        is.read_byte_order();
        if (is.read_uint32() != Code)
            throw std::runtime_error("WKB code mismatch");
        vi(tagged{});
        return vi(T::accept(is, vi), tagged{});
    }
};

using path = chain<vertex>;
using point = tagged<Point, vertex>;
using linestring = tagged<Linestring, path>;
using polygon = tagged<Polygon, chain<path>>;
using multi_point = tagged<MultiPoint, chain<point>>;
using multi_linestring = tagged<MultiLinestring, chain<linestring>>;
using multi_polygon = tagged<MultiPolygon, chain<polygon>>;
struct geometry;
using geometry_collection = tagged<GeometryCollection, chain<geometry>>;

struct geometry {
    template <typename T, typename Visitor>
    static auto do_accept(istream& is, Visitor& vi)
    {
        return vi(T::accept(is, vi), geometry{});
    }

    template <typename Visitor>
    static auto accept(istream& is, Visitor& vi)
    {
        auto look_ahead = is;
        look_ahead.read_byte_order();
        switch (look_ahead.read_uint32()) {
            case Point:
                return do_accept<point>(is, vi);
            case Linestring:
                return do_accept<linestring>(is, vi);
            case Polygon:
                return do_accept<polygon>(is, vi);
            case MultiPoint:
                return do_accept<multi_point>(is, vi);
            case MultiLinestring:
                return do_accept<multi_linestring>(is, vi);
            case MultiPolygon:
                return do_accept<multi_polygon>(is, vi);
            case GeometryCollection:
                return do_accept<geometry_collection>(is, vi);
            default:
                throw std::runtime_error("unsupported WKB code");
        }
    }
};

}  // namespace wkb
}  // namespace detail
}  // namespace bark

#endif  // BARK_DETAIL_WKB_VISITOR_HPP
