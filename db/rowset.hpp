// Andrew Naplavkov

#ifndef BARK_DB_ROWSET_HPP
#define BARK_DB_ROWSET_HPP

#include <bark/db/variant.hpp>
#include <bark/detail/unicode.hpp>
#include <cctype>
#include <string>

namespace bark::db {

/// An arrangement of @ref variant_t in rows and columns
struct rowset {
    std::vector<std::string> columns;
    blob data;
};

/// @return tuples of @ref variant_t
inline auto range(const rowset& that)
{
    auto res = std::vector<std::vector<variant_t>>{};
    for (auto is = variant_istream{that.data}; !is.data.empty();)
        for (auto& v : res.emplace_back(that.columns.size()))
            is >> v;
    return res;
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
    const std::vector<variant_t>& row;

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
    using row_t = std::vector<variant_t>;
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

template <class Rows, class Functor>
void for_each_blob(const Rows& rows, size_t col, Functor f)
{
    for (auto& row : rows)
        f(std::get<bark::blob_view>(row[col]));
}

}  // namespace bark::db

#endif  // BARK_DB_ROWSET_HPP
