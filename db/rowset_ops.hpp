// Andrew Naplavkov

#ifndef BARK_DB_ROWSET_OPS_HPP
#define BARK_DB_ROWSET_OPS_HPP

#include <bark/db/rowset.hpp>
#include <bark/unicode.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <cctype>
#include <stdexcept>

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

namespace bark::db {

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

inline auto range(const rowset& src)
{
    auto res = std::vector<row_t>{};
    for (auto is = variant_istream{src.data}; !is.data.empty();)
        for (auto& v : res.emplace_back(src.columns.size()))
            is >> v;
    return res;
}

inline bool is_null(const variant_t& v)
{
    return std::holds_alternative<std::monostate>(v);
}

inline bool test(const variant_t& v)
{
    return !is_null(v) && boost::lexical_cast<int64_t>(v);
}

namespace detail {

inline std::string format(const variant_t& src, const std::ostream& dest)
{
    std::ostringstream os;
    os.copyfmt(dest);
    os << src;
    auto res = os.str();
    std::replace_if(
        res.begin(), res.end(), [](char ch) { return std::isspace(ch); }, ' ');
    return res;
}

struct line {
    std::vector<size_t>& sizes;
    const row_t& row;

    void fit(const std::ostream& dest)
    {
        for (size_t i = 0; i < sizes.size(); ++i)
            sizes[i] = std::max(sizes[i], unicode::size(format(row[i], dest)));
    }

    friend std::ostream& operator<<(std::ostream& dest, const line& src)
    {
        for (size_t i = 0; i < src.sizes.size(); ++i) {
            auto str = format(src.row[i], dest);
            auto indent = src.sizes[i] - unicode::size(str);
            dest << "|" << std::setw(1) << "";
            if (std::holds_alternative<std::string_view>(src.row[i]))
                dest << str << std::setw(indent) << "";
            else
                dest << std::setw(indent) << "" << str;
            dest << std::setw(1) << "";
        }
        return dest << "|\n";
    }
};

}  // namespace detail

inline std::ostream& operator<<(std::ostream& dest, const rowset& src)
{
    auto columns = as<row_t>(src.columns);
    auto rows = range(src);
    auto sizes = std::vector<size_t>(columns.size());
    detail::line{sizes, columns}.fit(dest);
    for (auto& row : rows)
        detail::line{sizes, row}.fit(dest);
    dest << std::setfill(' ') << detail::line{sizes, columns}
         << std::setfill('-') << detail::line{sizes, row_t(columns.size())}
         << std::setfill(' ');
    for (auto& row : rows)
        dest << detail::line{sizes, row};
    return dest;
}

inline rowset select(std::vector<std::string> cols, const rowset& from)
{
    auto positions = as<std::vector<size_t>>(cols, [&](auto& lhs) {
        auto it = boost::range::find_if(from.columns, [&](auto& rhs) {
            return unicode::case_insensitive_equal_to{}(lhs, rhs);
        });
        return std::distance(from.columns.begin(), it);
    });
    variant_ostream os;
    for (auto& row : range(from))
        for (auto pos : positions)
            os << row.at(pos);
    return {std::move(cols), std::move(os.data)};
}

template <class Rows, class Functor>
void for_each_blob(const Rows& rows, size_t col, Functor f)
{
    for (auto& row : rows)
        f(std::get<bark::blob_view>(row[col]));
}

}  // namespace bark::db

#endif  // BARK_DB_ROWSET_OPS_HPP
