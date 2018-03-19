// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_DETAIL_BIND_PARAM_HPP
#define BARK_DB_MYSQL_DETAIL_BIND_PARAM_HPP

#include <bark/dataset/variant_view.hpp>
#include <bark/db/mysql/detail/common.hpp>
#include <boost/variant/static_visitor.hpp>
#include <cstring>
#include <type_traits>

namespace bark {
namespace db {
namespace mysql {
namespace detail {

class bind_param_visitor : public boost::static_visitor<void> {
    MYSQL_BIND& bnd_;

public:
    explicit bind_param_visitor(MYSQL_BIND& bnd) : bnd_(bnd)
    {
        memset(&bnd_, 0, sizeof(MYSQL_BIND));
    }

    void operator()(boost::blank) { bnd_.buffer_type = MYSQL_TYPE_NULL; }

    template <typename T>
    void operator()(std::reference_wrapper<const T> v)
    {
        bnd_.buffer_type = code_of<T>();
        bnd_.buffer = (char*)&v.get();
    }

    template <typename T>
    std::enable_if_t<is_same<T, boost::string_view, blob_view>()> operator()(
        T v)
    {
        bnd_.buffer_type = code_of<T>();
        bnd_.buffer = (char*)v.data();
        bnd_.buffer_length = (unsigned long)v.size();
    }
};

inline void bind_param(dataset::variant_view param, MYSQL_BIND& bnd)
{
    bind_param_visitor visitor(bnd);
    boost::apply_visitor(visitor, param);
}

}  // namespace detail
}  // namespace mysql
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_MYSQL_DETAIL_BIND_PARAM_HPP
