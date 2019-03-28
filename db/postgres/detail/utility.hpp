// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_UTILITY_HPP
#define BARK_DB_POSTGRES_UTILITY_HPP

#include <bark/blob.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/map.hpp>
#include <cstdint>
#include <libpq-fe.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace bark::db::postgres {

constexpr Oid PGRES_TYPE_BOOL = 16;
constexpr Oid PGRES_TYPE_INT2 = 21;
constexpr Oid PGRES_TYPE_INT4 = 23;
constexpr Oid PGRES_TYPE_INT8 = 20;
constexpr Oid PGRES_TYPE_FLOAT4 = 700;
constexpr Oid PGRES_TYPE_FLOAT8 = 701;
constexpr Oid PGRES_TYPE_BPCHAR = 1042;
constexpr Oid PGRES_TYPE_BPCHARARRAY = 1014;
constexpr Oid PGRES_TYPE_NAME = 19;
constexpr Oid PGRES_TYPE_TEXT = 25;
constexpr Oid PGRES_TYPE_TEXTARRAY = 1009;
constexpr Oid PGRES_TYPE_VARCHAR = 1043;
constexpr Oid PGRES_TYPE_VARCHARARRAY = 1015;
constexpr Oid PGRES_TYPE_BYTEA = 17;

constexpr int PGRES_FORMAT_TEXT = 0;
constexpr int PGRES_FORMAT_BINARY = 1;

template <class T>
constexpr auto code_of()
{
    using namespace boost::mpl;
    return (Oid)at<map<pair<int64_t, int_<PGRES_TYPE_INT8>>,
                       pair<double, int_<PGRES_TYPE_FLOAT8>>,
                       pair<std::string_view, int_<PGRES_TYPE_TEXT>>,
                       pair<blob_view, int_<PGRES_TYPE_BYTEA>>>,
                   T>::type::value;
}

struct connection_deleter {
    void operator()(PGconn* p) const { PQfinish(p); }
};

using connection_holder = std::unique_ptr<PGconn, connection_deleter>;

struct result_deleter {
    void operator()(PGresult* p) const { PQclear(p); }
};

using result_holder = std::unique_ptr<PGresult, result_deleter>;

inline std::string error(PGconn* con)
{
    std::string res;
    if (con)
        res = PQerrorMessage(con);
    if (res.empty())
        res = "Postgres error";
    return res;
}

inline void check(const connection_holder& con, bool condition)
{
    if (!condition)
        throw std::runtime_error(error(con.get()));
}

}  // namespace bark::db::postgres

#endif  // BARK_DB_POSTGRES_UTILITY_HPP
