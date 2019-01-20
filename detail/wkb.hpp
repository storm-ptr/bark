// Andrew Naplavkov

/// @see https://en.wikipedia.org/wiki/Well-known_text#Well-known_binary

#ifndef BARK_WKB_HPP
#define BARK_WKB_HPP

#include <bark/blob.hpp>
#include <boost/predef/other/endian.h>
#include <cstdint>
#include <stdexcept>

namespace bark::wkb {

constexpr uint8_t BigEndian = 0;
constexpr uint8_t LittleEndian = 1;
constexpr uint8_t HostEndian =
#if defined BOOST_ENDIAN_LITTLE_BYTE
    LittleEndian;
#elif defined BOOST_ENDIAN_BIG_BYTE
    BigEndian;
#else
#error WKB byte order
#endif

constexpr uint32_t Point = 1;
constexpr uint32_t Linestring = 2;
constexpr uint32_t Polygon = 3;
constexpr uint32_t MultiPoint = 4;
constexpr uint32_t MultiLinestring = 5;
constexpr uint32_t MultiPolygon = 6;
constexpr uint32_t GeometryCollection = 7;

class istream {
    blob_view data_;
    uint8_t endian_;

public:
    explicit istream(blob_view data) : data_{data}, endian_{HostEndian} {}

    uint8_t read_byte_order()
    {
        endian_ = read<uint8_t>(data_);
        return (endian_ == BigEndian || endian_ == LittleEndian)
                   ? endian_
                   : throw std::runtime_error("invalid WKB byte order");
    }

    uint32_t read_uint32()
    {
        auto res = read<uint32_t>(data_);
        if (HostEndian != endian_)
            res = reversed(res);
        return res ? res : throw std::runtime_error("unsupported WKB value");
    }

    double read_double()
    {
        auto res = read<double>(data_);
        if (HostEndian != endian_)
            res = reversed(res);
        return res;
    }
};

struct vertex {
    template <class Visitor>
    static auto accept(istream& is, Visitor& vis)
    {
        auto x = is.read_double();
        auto y = is.read_double();
        return vis(x, y);
    }
};

template <class T>
struct chain {
    template <class Visitor>
    static auto accept(istream& is, Visitor& vis)
    {
        auto count = is.read_uint32();
        auto sum = vis(count, chain{});
        for (decltype(count) i = 0; i < count; ++i)
            vis(sum, T::accept(is, vis), chain{});
        return sum;
    }
};

template <uint32_t Code, class T>
struct tagged {
    template <class Visitor>
    static auto accept(istream& is, Visitor& vis)
    {
        is.read_byte_order();
        if (is.read_uint32() != Code)
            throw std::runtime_error("WKB code mismatch");
        vis(tagged{});
        return vis(T::accept(is, vis), tagged{});
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
    template <class T, class Visitor>
    static auto do_accept(istream& is, Visitor& vis)
    {
        return vis(T::accept(is, vis), geometry{});
    }

    template <class Visitor>
    static auto accept(istream& is, Visitor& vis)
    {
        auto look_ahead = is;
        look_ahead.read_byte_order();
        switch (look_ahead.read_uint32()) {
            case Point:
                return do_accept<point>(is, vis);
            case Linestring:
                return do_accept<linestring>(is, vis);
            case Polygon:
                return do_accept<polygon>(is, vis);
            case MultiPoint:
                return do_accept<multi_point>(is, vis);
            case MultiLinestring:
                return do_accept<multi_linestring>(is, vis);
            case MultiPolygon:
                return do_accept<multi_polygon>(is, vis);
            case GeometryCollection:
                return do_accept<geometry_collection>(is, vis);
        }
        throw std::runtime_error("unsupported WKB code");
    }
};

}  // namespace bark::wkb

#endif  // BARK_WKB_HPP
