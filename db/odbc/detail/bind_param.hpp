// Andrew Naplavkov

#ifndef BARK_DB_ODBC_BIND_PARAM_HPP
#define BARK_DB_ODBC_BIND_PARAM_HPP

#include <bark/db/odbc/detail/utility.hpp>
#include <bark/db/rowset.hpp>
#include <bark/detail/unicode.hpp>
#include <memory>

namespace bark::db::odbc {

struct binding {
    virtual ~binding() = default;
    virtual SQLSMALLINT input_output_type() = 0;
    virtual SQLSMALLINT c_type() = 0;
    virtual SQLSMALLINT sql_type() = 0;
    virtual SQLPOINTER val_ptr() = 0;
    virtual SQLLEN* ind() = 0;
};

using binding_holder = std::unique_ptr<binding>;

inline SQLULEN column_size(binding& bnd)
{
    switch (bnd.c_type()) {
        case SQL_C_FLOAT:
            return 7;
        case SQL_C_DOUBLE:
            return 15;
        default:
            return bnd.ind() ? std::max<SQLULEN>(*bnd.ind(), 1) : 0;
    }
}

class binding_stub : public binding {
public:
    SQLSMALLINT input_output_type() override { return SQL_PARAM_OUTPUT; }
    SQLSMALLINT c_type() override { return SQL_C_CHAR; }
    SQLSMALLINT sql_type() override { return SQL_CHAR; }
    SQLPOINTER val_ptr() override { return nullptr; }
    SQLLEN* ind() override { return nullptr; }
};

class binding_null : public binding {
    SQLLEN ind_ = SQL_NULL_DATA;

public:
    SQLSMALLINT input_output_type() override { return SQL_PARAM_INPUT; }
    SQLSMALLINT c_type() override { return SQL_C_CHAR; }
    SQLSMALLINT sql_type() override { return SQL_CHAR; }
    SQLPOINTER val_ptr() override { return nullptr; }
    SQLLEN* ind() override { return &ind_; }
};

template <class T>
class binding_val : public binding {
    T val_;
    SQLLEN ind_ = sizeof(T);

public:
    explicit binding_val(T v) : val_(v) {}
    SQLSMALLINT input_output_type() override { return SQL_PARAM_INPUT; }
    SQLSMALLINT c_type() override { return c_type_of<T>(); }
    SQLSMALLINT sql_type() override { return sql_type_of<T>(); }
    SQLPOINTER val_ptr() override { return &val_; }
    SQLLEN* ind() override { return &ind_; }
};

class binding_text : public binding {
    std::basic_string<SQLWCHAR> val_;
    SQLLEN ind_;

public:
    explicit binding_text(std::string_view v)
        : val_(unicode::to_string<SQLWCHAR>(v))
        , ind_(val_.size() * sizeof(SQLWCHAR))
    {
    }
    SQLSMALLINT input_output_type() override { return SQL_PARAM_INPUT; }
    SQLSMALLINT c_type() override { return SQL_C_WCHAR; }
    SQLSMALLINT sql_type() override { return SQL_WVARCHAR; }
    SQLPOINTER val_ptr() override { return SQLPOINTER(val_.c_str()); }
    SQLLEN* ind() override { return &ind_; }
};

class binding_blob : public binding {
    SQLPOINTER ptr_;
    SQLLEN ind_;

public:
    explicit binding_blob(blob_view v) : ptr_((void*)v.data()), ind_(v.size())
    {
    }
    SQLSMALLINT input_output_type() override { return SQL_PARAM_INPUT; }
    SQLSMALLINT c_type() override { return SQL_C_BINARY; }
    SQLSMALLINT sql_type() override { return SQL_VARBINARY; }
    SQLPOINTER val_ptr() override { return ptr_; }
    SQLLEN* ind() override { return &ind_; }
};

inline binding_holder bind_param(const variant_t* v)
{
    if (v)
        return std::visit(
            overloaded{[](std::monostate) -> binding_holder {
                           return std::make_unique<binding_null>();
                       },
                       [](auto v) -> binding_holder {
                           return std::make_unique<binding_val<decltype(v)>>(v);
                       },
                       [](std::string_view v) -> binding_holder {
                           return std::make_unique<binding_text>(v);
                       },
                       [](blob_view v) -> binding_holder {
                           return std::make_unique<binding_blob>(v);
                       }},
            *v);
    else
        return std::make_unique<binding_stub>();
}

}  // namespace bark::db::odbc

#endif  // BARK_DB_ODBC_BIND_PARAM_HPP
