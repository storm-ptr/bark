// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_BIND_COLUMN_HPP
#define BARK_DB_POSTGRES_BIND_COLUMN_HPP

#include <bark/db/postgres/detail/utility.hpp>
#include <bark/db/variant.hpp>
#include <boost/predef/other/endian.h>
#include <stdexcept>

namespace bark::db::postgres {

struct column {
    virtual ~column() = default;
    virtual void write(PGresult*, int row, int col, variant_ostream&) = 0;
};

using column_holder = std::unique_ptr<column>;

template <class T>
struct column_val : column {
    void write(PGresult* rows, int row, int col, variant_ostream& os) override
    {
        auto val = *(const T*)PQgetvalue(rows, row, col);
#if defined BOOST_ENDIAN_LITTLE_BYTE
        os << reversed(val);
#elif defined BOOST_ENDIAN_BIG_BYTE
        os << val;
#else
#error byte order error
#endif
    }
};

template <class T>
struct column_arr : column {
    void write(PGresult* rows, int row, int col, variant_ostream& os) override
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
            return std::make_unique<column_arr<std::string_view>>();
        case PGRES_TYPE_BYTEA:
            return std::make_unique<column_arr<blob_view>>();
    }
    throw std::runtime_error("invalid Postgres type: " + std::to_string(type));
}

}  // namespace bark::db::postgres

#endif  // BARK_DB_POSTGRES_BIND_COLUMN_HPP
