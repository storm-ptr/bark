// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_BIND_PARAM_HPP
#define BARK_DB_MYSQL_BIND_PARAM_HPP

#include <bark/db/mysql/detail/utility.hpp>
#include <bark/db/variant.hpp>
#include <cstring>

namespace bark::db::mysql {

inline void bind_param(const variant_t& v, MYSQL_BIND& bnd)
{
    memset(&bnd, 0, sizeof(MYSQL_BIND));
    std::visit(
        overloaded{[&](std::monostate) { bnd.buffer_type = MYSQL_TYPE_NULL; },
                   [&](const int64_t& v) {
                       bnd.buffer_type = code_of<int64_t>();
                       bnd.buffer = (char*)&v;
                   },
                   [&](const double& v) {
                       bnd.buffer_type = code_of<double>();
                       bnd.buffer = (char*)&v;
                   },
                   [&](auto v) {
                       bnd.buffer_type = code_of<decltype(v)>();
                       bnd.buffer = (char*)v.data();
                       bnd.buffer_length = (unsigned long)v.size();
                   }},
        v);
}

}  // namespace bark::db::mysql

#endif  // BARK_DB_MYSQL_BIND_PARAM_HPP
