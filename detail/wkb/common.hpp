// Andrew Naplavkov

/// @see https://en.wikipedia.org/wiki/Well-known_text#Well-known_binary

#ifndef BARK_DETAIL_WKB_COMMON_HPP
#define BARK_DETAIL_WKB_COMMON_HPP

#include <boost/detail/endian.hpp>
#include <cstdint>

namespace bark {
namespace detail {
namespace wkb {

constexpr uint8_t BigEndian = 0;
constexpr uint8_t LittleEndian = 1;
#if defined BOOST_LITTLE_ENDIAN
constexpr uint8_t HostEndian = LittleEndian;
#elif defined BOOST_BIG_ENDIAN
constexpr uint8_t HostEndian = BigEndian;
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

}  // namespace wkb
}  // namespace detail

namespace wkb = detail::wkb;

}  // namespace bark

#endif  // BARK_DETAIL_WKB_COMMON_HPP
