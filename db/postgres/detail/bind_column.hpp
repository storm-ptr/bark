// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_DETAIL_BIND_COLUMN_HPP
#define BARK_DB_POSTGRES_DETAIL_BIND_COLUMN_HPP

#include <bark/dataset/ostream.hpp>
#include <bark/db/postgres/detail/common.hpp>
#include <boost/detail/endian.hpp>
#include <stdexcept>

namespace bark {
namespace db {
namespace postgres {
namespace detail {

struct column {
    virtual ~column() = default;
    virtual void write(PGresult*, int row, int col, dataset::ostream&) = 0;
};

using column_holder = std::unique_ptr<column>;

template <typename T>
struct column_val : column {
    void write(PGresult* rows, int row, int col, dataset::ostream& os) override
    {
        auto val = *(const T*)PQgetvalue(rows, row, col);
#if defined BOOST_LITTLE_ENDIAN
        os << reversed(val);
#elif defined BOOST_BIG_ENDIAN
        os << val;
#else
#error byte order error
#endif
    }
};

template <typename T>
struct column_arr : column {
    void write(PGresult* rows, int row, int col, dataset::ostream& os) override
    {
        os << T{(const typename T::value_type*)PQgetvalue(rows, row, col),
                (size_t)PQgetlength(rows, row, col)};
    }
};

inline column_holder bind_column(Oid type)
{
    switch (type) {
        case PGRES_TYPE_BOOL:
            return std::make_unique<column_val<int8_t>>();
        case PGRES_TYPE_INT2:
            return std::make_unique<column_val<int16_t>>();
        case PGRES_TYPE_INT4:
            return std::make_unique<column_val<int32_t>>();
        case PGRES_TYPE_INT8:
            return std::make_unique<column_val<int64_t>>();
        case PGRES_TYPE_FLOAT4:
            return std::make_unique<column_val<float>>();
        case PGRES_TYPE_FLOAT8:
            return std::make_unique<column_val<double>>();
        case PGRES_TYPE_BPCHAR:
        case PGRES_TYPE_BPCHARARRAY:
        case PGRES_TYPE_NAME:
        case PGRES_TYPE_TEXT:
        case PGRES_TYPE_TEXTARRAY:
        case PGRES_TYPE_VARCHAR:
        case PGRES_TYPE_VARCHARARRAY:
            return std::make_unique<column_arr<boost::string_view>>();
        case PGRES_TYPE_BYTEA:
            return std::make_unique<column_arr<blob_view>>();
        default:
            throw std::runtime_error("Postgres type error: " +
                                     std::to_string(type));
    }
}

}  // namespace detail
}  // namespace postgres
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_POSTGRES_DETAIL_BIND_COLUMN_HPP
