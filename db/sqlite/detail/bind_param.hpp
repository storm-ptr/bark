// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_DETAIL_BIND_PARAM_HPP
#define BARK_DB_SQLITE_DETAIL_BIND_PARAM_HPP

#include <bark/dataset/variant_view.hpp>
#include <bark/db/sqlite/detail/common.hpp>
#include <boost/variant/static_visitor.hpp>
#include <type_traits>

namespace bark {
namespace db {
namespace sqlite {
namespace detail {

struct bind_param_visitor : boost::static_visitor<int> {
    sqlite3_stmt* stmt;
    int order;

    int operator()(boost::blank) { return sqlite3_bind_null(stmt, order); }

    template <typename T>
    std::enable_if_t<std::is_integral<T>::value, int> operator()(
        std::reference_wrapper<const T> v)
    {
        return sqlite3_bind_int64(stmt, order, int64_t(v.get()));
    }

    template <typename T>
    std::enable_if_t<std::is_floating_point<T>::value, int> operator()(
        std::reference_wrapper<const T> v)
    {
        return sqlite3_bind_double(stmt, order, double(v.get()));
    }

    int operator()(boost::string_view v)
    {
        return sqlite3_bind_text(
            stmt, order, v.data(), int(v.size()), SQLITE_TRANSIENT);
    }

    int operator()(blob_view v)
    {
        return sqlite3_bind_blob(
            stmt, order, v.data(), int(v.size()), SQLITE_TRANSIENT);
    }
};

inline int bind_param(dataset::variant_view param,
                      sqlite3_stmt* stmt,
                      size_t order)
{
    bind_param_visitor visitor;
    visitor.stmt = stmt;
    visitor.order = int(order + 1);  // one-based
    return boost::apply_visitor(visitor, param);
}

}  // namespace detail
}  // namespace sqlite
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SQLITE_DETAIL_BIND_PARAM_HPP
