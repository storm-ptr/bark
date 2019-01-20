// Andrew Naplavkov

#ifndef BARK_DB_ODBC_DETAIL_UTILITY_HPP
#define BARK_DB_ODBC_DETAIL_UTILITY_HPP

#include <bark/unicode.hpp>
#include <bark/utility.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/map.hpp>
#include <iostream>
#include <memory>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <stdexcept>
#include <string>

namespace bark::db::odbc::detail {

template <class T>
constexpr SQLSMALLINT c_type_of()
{
    using namespace boost::mpl;
    return at<map<pair<int64_t, int_<SQL_C_SBIGINT>>,
                  pair<double, int_<SQL_C_DOUBLE>>>,
              T>::type::value;
}

template <class T>
constexpr SQLSMALLINT sql_type_of()
{
    using namespace boost::mpl;
    return at<
        map<pair<int64_t, int_<SQL_BIGINT>>, pair<double, int_<SQL_DOUBLE>>>,
        T>::type::value;
}

struct env_deleter {
    static constexpr SQLSMALLINT handle_type = SQL_HANDLE_ENV;

    void operator()(void* p) const { SQLFreeHandle(handle_type, p); }
};

using env_holder = std::unique_ptr<void, env_deleter>;

struct dbc_deleter {
    static constexpr SQLSMALLINT handle_type = SQL_HANDLE_DBC;

    void operator()(void* p) const
    {
        SQLDisconnect(p);
        SQLFreeHandle(handle_type, p);
    }
};

using dbc_holder = std::unique_ptr<void, dbc_deleter>;

struct stmt_deleter {
    static constexpr SQLSMALLINT handle_type = SQL_HANDLE_STMT;

    void operator()(void* p) const
    {
        SQLFreeStmt(p, SQL_CLOSE);
        SQLFreeHandle(handle_type, p);
    }
};

using stmt_holder = std::unique_ptr<void, stmt_deleter>;

template <size_t Size>
struct buffer {
    static constexpr size_t max_size = Size;

    SQLWCHAR data[max_size] = {0};
    SQLSMALLINT len = 0;

    std::string to_string() const { return unicode::to_string<char>(data); }
};

template <class HandleHolder>
void check(const HandleHolder& handle, SQLRETURN r)
{
    std::string msg;
    if (!!handle && (SQL_ERROR == r || SQL_SUCCESS_WITH_INFO == r)) {
        buffer<6> state;
        buffer<SQL_MAX_MESSAGE_LENGTH> buf;
        SQLINTEGER err = 0;
        for (SQLSMALLINT row = 1; SQL_SUCCEEDED(
                 SQLGetDiagRecW(HandleHolder::deleter_type::handle_type,
                                handle.get(),
                                row,
                                state.data,
                                &err,
                                buf.data,
                                buf.max_size,
                                &buf.len));
             ++row) {
            if (!msg.empty())
                msg += ' ';
            msg += buf.to_string();
        }
    }
    if (!SQL_SUCCEEDED(r))
        throw std::runtime_error(msg.empty() ? "ODBC error" : msg);
    if (!msg.empty())
        std::cout << msg << std::endl;
}

template <class HandleHolder>
SQLHANDLE alloc_handle(const HandleHolder& handle, SQLSMALLINT HandleType)
{
    SQLHANDLE res = SQL_NULL_HANDLE;
    check(handle, SQLAllocHandle(HandleType, handle.get(), &res));
    return res;
}

inline void set_attr(const env_holder& handle, SQLINTEGER attr, intptr_t val)
{
    check(handle, SQLSetEnvAttr(handle.get(), attr, SQLPOINTER(val), 0));
}

inline void set_attr(const dbc_holder& handle, SQLINTEGER attr, intptr_t val)
{
    check(handle, SQLSetConnectAttr(handle.get(), attr, SQLPOINTER(val), 0));
}

inline void set_attr(const stmt_holder& handle, SQLINTEGER attr, intptr_t val)
{
    check(handle, SQLSetStmtAttr(handle.get(), attr, SQLPOINTER(val), 0));
}

inline std::string get_info(const dbc_holder& handle, SQLUSMALLINT info_type)
{
    buffer<SQL_MAX_MESSAGE_LENGTH> buf;
    check(
        handle,
        SQLGetInfoW(handle.get(), info_type, buf.data, buf.max_size, &buf.len));
    return buf.to_string();
}

inline std::string string_attr(const stmt_holder& handle,
                               SQLUSMALLINT col,
                               SQLUSMALLINT attr)
{
    buffer<SQL_MAX_MESSAGE_LENGTH> buf;
    check(handle,
          SQLColAttributeW(
              handle.get(), col, attr, buf.data, buf.max_size, &buf.len, 0));
    return buf.to_string();
}

inline SQLLEN numeric_attr(const stmt_holder& handle,
                           SQLUSMALLINT col,
                           SQLUSMALLINT attr)
{
    SQLLEN res = 0;
    check(handle, SQLColAttributeW(handle.get(), col, attr, 0, 0, 0, &res));
    return res;
}

constexpr SQLSMALLINT SQL_DB2_UNICODE_CHAR = -95;
constexpr SQLSMALLINT SQL_DB2_UNICODE_VARCHAR = -96;
constexpr SQLSMALLINT SQL_DB2_UNICODE_LONGVARCHAR = -97;
constexpr SQLSMALLINT SQL_DB2_BLOB = -98;
constexpr SQLSMALLINT SQL_DB2_CLOB = -99;

}  // namespace bark::db::odbc::detail

#endif  // BARK_DB_ODBC_DETAIL_UTILITY_HPP
