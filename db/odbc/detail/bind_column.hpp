// Andrew Naplavkov

#ifndef BARK_DB_ODBC_BIND_COLUMN_HPP
#define BARK_DB_ODBC_BIND_COLUMN_HPP

#include <bark/db/odbc/detail/utility.hpp>
#include <bark/detail/unicode.hpp>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace bark::db::odbc {

struct column {
    virtual ~column() = default;
    virtual SQLRETURN write(SQLHSTMT stmt,
                            SQLUSMALLINT col,
                            variant_ostream& os) = 0;
};

using column_holder = std::unique_ptr<column>;

template <class T>
class column_val : public column {
public:
    SQLRETURN write(SQLHSTMT stmt,
                    SQLUSMALLINT col,
                    variant_ostream& os) override
    {
        T val{};
        SQLLEN ind = SQL_NULL_DATA;
        auto r = SQLGetData(stmt, col, c_type_of<T>(), &val, sizeof(val), &ind);
        if (!SQL_SUCCEEDED(r) || SQL_NULL_DATA == ind)
            os << variant_t{};
        else
            os << val;
        return r;
    }
};

struct get_data_piecewise {
    SQLLEN ind_ = SQL_NULL_DATA;
    SQLLEN len_ = 0;
    blob data_;

    get_data_piecewise() : data_(SQL_MAX_MESSAGE_LENGTH) {}

    SQLRETURN fetch(SQLHSTMT stmt, SQLUSMALLINT col, SQLSMALLINT TargetType)
    {
        len_ = 0;
        while (true) {
            auto rest = (SQLLEN)data_.size() - len_;
            ind_ = 0;
            auto r = SQLGetData(
                stmt, col, TargetType, data_.data() + len_, rest, &ind_);

            if (r == SQL_NO_DATA) {
                ind_ = 0;
                break;
            }
            else if (!SQL_SUCCEEDED(r) || ind_ == SQL_NULL_DATA) {
                return r;
            }
            else if (ind_ == SQL_NO_TOTAL) {
                ind_ = rest + SQL_MAX_MESSAGE_LENGTH;
            }
            else if (ind_ < 0) {
                return SQL_ERROR;
            }
            else if (ind_ <= rest) {
                break;
            }

            len_ = data_.size();
            data_.resize(data_.size() + ind_ - rest);
        }
        len_ += ind_;
        return SQL_SUCCESS;
    }
};

class column_blob : public column, private get_data_piecewise {
public:
    SQLRETURN write(SQLHSTMT stmt,
                    SQLUSMALLINT col,
                    variant_ostream& os) override
    {
        auto r = fetch(stmt, col, SQL_C_BINARY);
        if (SQL_SUCCEEDED(r)) {
            if (SQL_NULL_DATA == ind_)
                os << variant_t{};
            else
                os << blob_view{data_.data(), (size_t)len_};
        }
        return r;
    }
};

class column_text : public column, private get_data_piecewise {
public:
    SQLRETURN write(SQLHSTMT stmt,
                    SQLUSMALLINT col,
                    variant_ostream& os) override
    {
        auto r = fetch(stmt, col, SQL_C_WCHAR);
        if (SQL_SUCCEEDED(r)) {
            if (SQL_NULL_DATA == ind_)
                os << variant_t{};
            else
                os << unicode::to_string<char>(std::basic_string_view<SQLWCHAR>{
                    (SQLWCHAR*)data_.data(), (size_t)len_ / sizeof(SQLWCHAR)});
        }
        return r;
    }
};

inline SQLSMALLINT sql_type(const stmt_holder& stmt, SQLUSMALLINT col)
{
    auto res = numeric_attr(stmt, col, SQL_DESC_TYPE);
    if (SQL_DECIMAL == res || SQL_NUMERIC == res) {
        auto precision = numeric_attr(stmt, col, SQL_DESC_PRECISION);
        auto scale = numeric_attr(stmt, col, SQL_DESC_SCALE);
        if (0 < precision && 0 == scale) {
            if (precision <= 5)
                res = SQL_SMALLINT;
            else if (precision <= 10)
                res = SQL_INTEGER;
            else if (precision <= 19)
                res = SQL_BIGINT;
        }
    }
    return SQLSMALLINT(res);
}

inline column_holder bind_column(const stmt_holder& stmt, SQLUSMALLINT col)
{
    auto type = sql_type(stmt, col);
    switch (type) {
        case SQL_BIT:
        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
            return std::make_unique<column_val<int64_t>>();
        case SQL_REAL:
        case SQL_DECIMAL:
        case SQL_DOUBLE:
        case SQL_FLOAT:
        case SQL_NUMERIC:
            return std::make_unique<column_val<double>>();
        case SQL_DATE:
        case SQL_TIME:
        case SQL_TIMESTAMP:
        case SQL_TYPE_DATE:
        case SQL_TYPE_TIME:
        case SQL_TYPE_TIMESTAMP:
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
        case SQL_DB2_UNICODE_CHAR:
        case SQL_DB2_UNICODE_VARCHAR:
        case SQL_DB2_UNICODE_LONGVARCHAR:
        case SQL_DB2_CLOB:
            return std::make_unique<column_text>();
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
        case SQL_DB2_BLOB:
            return std::make_unique<column_blob>();
    }
    throw std::runtime_error("invalid ODBC type: " + std::to_string(type));
}

}  // namespace bark::db::odbc

#endif  // BARK_DB_ODBC_BIND_COLUMN_HPP
