// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_BIND_COLUMN_HPP
#define BARK_DB_MYSQL_BIND_COLUMN_HPP

#include <bark/db/mysql/detail/utility.hpp>

namespace bark::db::mysql {

struct column {
    virtual ~column() = default;
    virtual bool resize() = 0;
    virtual void write(variant_ostream&) = 0;
};

using column_holder = std::unique_ptr<column>;

template <class T>
class column_val : public column {
    bool_t is_null_ = 0;
    T val_ = 0;

public:
    explicit column_val(MYSQL_BIND& bnd)
    {
        bnd.is_null = &is_null_;
        bnd.buffer_type = code_of<T>();
        bnd.buffer = (void*)&val_;
        bnd.buffer_length = sizeof(T);
    }

    bool resize() override { return false; }

    void write(variant_ostream& os) override
    {
        if (is_null_)
            os << variant_t{};
        else
            os << val_;
    }
};

template <class T>
class column_arr : public column {
    MYSQL_BIND& bnd_;
    bool_t is_null_ = 0;
    unsigned long len_ = 0;
    blob buf_;

public:
    explicit column_arr(MYSQL_BIND& bnd) : bnd_(bnd)
    {
        bnd_.is_null = &is_null_;
        bnd_.buffer_type = code_of<T>();
        bnd_.length = &len_;
    }

    bool resize() override
    {
        if (!is_null_ && bnd_.buffer_length < len_) {
            buf_.resize(len_);
            bnd_.buffer = (void*)buf_.data();
            bnd_.buffer_length = len_;
        }
        return true;
    }

    void write(variant_ostream& os) override
    {
        if (is_null_)
            os << variant_t{};
        else
            os << T{(typename T::value_type*)buf_.data(), (size_t)len_};
    }
};

inline column_holder bind_column(enum_field_types type,
                                 unsigned charsetnr,
                                 MYSQL_BIND& bnd)
{
    switch (type) {
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_LONGLONG:
            return std::make_unique<column_val<int64_t>>(bnd);
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_NEWDECIMAL:
            return std::make_unique<column_val<double>>(bnd);
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_TINY_BLOB:
            if (63 == charsetnr)
                return std::make_unique<column_arr<blob_view>>(bnd);
            [[fallthrough]];
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_NULL:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_YEAR:
            return std::make_unique<column_arr<std::string_view>>(bnd);
        default:
            throw std::runtime_error(concat("unsupported MySQL type: ", type));
    }
}

}  // namespace bark::db::mysql

#endif  // BARK_DB_MYSQL_BIND_COLUMN_HPP
