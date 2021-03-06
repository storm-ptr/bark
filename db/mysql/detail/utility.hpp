// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_UTILITY_HPP
#define BARK_DB_MYSQL_UTILITY_HPP

#ifdef _WIN32
#include <winsock2.h>
// strictly ordered
#include <windows.h>
#endif

#include <bark/blob.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/map.hpp>
#include <cstdint>
#include <memory>
#include <mysql.h>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace bark::db::mysql {

using bool_t = std::remove_pointer_t<decltype(MYSQL_BIND::is_null)>;

template <class T>
constexpr auto code_of()
{
    using namespace boost::mpl;
    return (
        enum_field_types)at<map<pair<int64_t, int_<MYSQL_TYPE_LONGLONG>>,
                                pair<double, int_<MYSQL_TYPE_DOUBLE>>,
                                pair<std::string_view, int_<MYSQL_TYPE_STRING>>,
                                pair<blob_view, int_<MYSQL_TYPE_BLOB>>>,
                            T>::type::value;
}

struct connection_deleter {
    void operator()(MYSQL* p) const { mysql_close(p); }
};

using connection_holder = std::unique_ptr<MYSQL, connection_deleter>;

struct statement_deleter {
    void operator()(MYSQL_STMT* p) const { mysql_stmt_close(p); }
};

using statement_holder = std::unique_ptr<MYSQL_STMT, statement_deleter>;

struct result_deleter {
    void operator()(MYSQL_RES* p) const { mysql_free_result(p); }
};

using result_holder = std::unique_ptr<MYSQL_RES, result_deleter>;

inline std::string error(MYSQL* p)
{
    return mysql_error(p);
}

inline std::string error(MYSQL_STMT* p)
{
    return mysql_stmt_error(p);
}

template <class HandleHolder>
void check(const HandleHolder& handle, bool condition)
{
    if (!condition) {
        std::string msg;
        if (handle)
            msg = error(handle.get());
        if (msg.empty())
            msg = "MySQL error";
        throw std::runtime_error(msg);
    }
}

}  // namespace bark::db::mysql

#endif  // BARK_DB_MYSQL_UTILITY_HPP
