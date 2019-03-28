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
    static auto accept(istream& is, Visitor& viz)
    {
        auto x = is.read_double();
        auto y = is.read_double();
        return viz(x, y);
    }
};

template <class T>
struct chain {
    template <class Visitor>
    static auto accept(istream& is, Visitor& viz)
    {
        auto count = is.read_uint32();
        auto res = viz(count, chain{});
        for (decltype(count) i = 0; i < count; ++i)
            viz(res, T::accept(is, viz), chain{});
        return res;
    }
};

template <class T, uint32_t Code>
struct tagged {
    template <class Visitor>
    static auto accept(istream& is, Visitor& viz)
    {
        is.read_byte_order();
        if (is.read_uint32() != Code)
            throw std::runtime_error("WKB code mismatch");
        viz(tagged{});
        return viz(T::accept(is, viz), tagged{});
    }
};

using path = chain<vertex>;
using point = tagged<vertex, Point>;
using linestring = tagged<path, Linestring>;
using polygon = tagged<chain<path>, Polygon>;
using multi_point = tagged<chain<point>, MultiPoint>;
using multi_linestring = tagged<chain<linestring>, MultiLinestring>;
using multi_polygon = tagged<chain<polygon>, MultiPolygon>;
struct geometry;
using geometry_collection = tagged<chain<geometry>, GeometryCollection>;

struct geometry {
    template <class Visitor>
    static auto accept(istream& is, Visitor& viz)
    {
        auto look_ahead = is;
        look_ahead.read_byte_order();
        switch (look_ahead.read_uint32()) {
            case Point:
                return viz(point::accept(is, viz), geometry{});
            case Linestring:
                return viz(linestring::accept(is, viz), geometry{});
            case Polygon:
                return viz(polygon::accept(is, viz), geometry{});
            case MultiPoint:
                return viz(multi_point::accept(is, viz), geometry{});
            case MultiLinestring:
                return viz(multi_linestring::accept(is, viz), geometry{});
            case MultiPolygon:
                return viz(multi_polygon::accept(is, viz), geometry{});
            case GeometryCollection:
                return viz(geometry_collection::accept(is, viz), geometry{});
            default:
                throw std::runtime_error("unsupported WKB code");
        }
    }
};

}  // namespace bark::wkb

#endif  // BARK_WKB_HPP
