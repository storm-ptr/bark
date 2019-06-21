// Andrew Naplavkov

#ifndef BARK_DB_VARIANT_HPP
#define BARK_DB_VARIANT_HPP

#include <bark/blob.hpp>
#include <cstdint>

namespace bark {
// @see https://github.com/doxygen/doxygen/issues/6765
namespace db {

/// Acts like a union for the most common types.
using variant_t =
    std::variant<std::monostate, int64_t, double, std::string_view, blob_view>;

/// Provides serialization of @ref variant_t
struct variant_istream {
    blob_view data;
};

/// Provides serialization of @ref variant_t
struct variant_ostream {
    blob data;
};

inline variant_t read(variant_istream& src)
{
    switch (bark::read<uint8_t>(src.data)) {
        case variant_index<variant_t, std::monostate>():
            return {};
        case variant_index<variant_t, int64_t>():
            return bark::read<int64_t>(src.data);
        case variant_index<variant_t, double>():
            return bark::read<double>(src.data);
        case variant_index<variant_t, std::string_view>(): {
            auto res = bark::read<std::string_view>(src.data);
            bark::read<char>(src.data);  // zero terminated
            return res;
        }
        case variant_index<variant_t, blob_view>():
            return bark::read<blob_view>(src.data);
    }
    throw std::logic_error{"invalid variant"};
}

inline void write(const variant_t& src, variant_ostream& dest)
{
    dest.data << static_cast<uint8_t>(src.index());
    std::visit(overloaded{[&](std::monostate) {},
                          [&](auto v) { dest.data << v; },
                          [&](std::string_view v) { dest.data << v << '\0'; }},
               src);
}

inline variant_istream& operator>>(variant_istream& src, variant_t& dest)
{
    dest = read(src);
    return src;
}

inline variant_ostream& operator<<(variant_ostream& dest, const variant_t& src)
{
    write(src, dest);
    return dest;
}

template <class T>
if_arithmetic_t<T, variant_ostream&> operator<<(variant_ostream& dest, T src)
{
    if constexpr (std::is_floating_point_v<T>)
        write((double)src, dest);
    else
        write((int64_t)src, dest);
    return dest;
}

inline bool is_null(const variant_t& v)
{
    return std::holds_alternative<std::monostate>(v);
}

}  // namespace db
}  // namespace bark

namespace std {

inline std::ostream& operator<<(std::ostream& dest,
                                const bark::db::variant_t& src)
{
    std::visit(
        bark::overloaded{[&](std::monostate) { dest << ""; },
                         [&](auto v) { dest << v; },
                         [&](bark::blob_view v) {
                             // call stream once
                             dest << (std::to_string(v.size()) + " bytes");
                         }},
        src);
    return dest;
}

}  // namespace std

#endif  // BARK_DB_VARIANT_HPP
