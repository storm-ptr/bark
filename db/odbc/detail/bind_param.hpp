// Andrew Naplavkov

#ifndef BARK_DB_ODBC_DETAIL_BIND_PARAM_HPP
#define BARK_DB_ODBC_DETAIL_BIND_PARAM_HPP

#include <bark/dataset/variant_view.hpp>
#include <bark/db/odbc/detail/common.hpp>
#include <bark/unicode.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/variant/static_visitor.hpp>
#include <memory>

namespace bark {
namespace db {
namespace odbc {
namespace detail {

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

template <typename T>
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
    explicit binding_text(boost::string_view v)
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

class binding_visitor : public boost::static_visitor<binding_holder> {
public:
    binding_holder operator()(const boost::blank&) const
    {
        return std::make_unique<binding_null>();
    }

    template <typename T>
    binding_holder operator()(std::reference_wrapper<const T> v) const
    {
        return std::make_unique<binding_val<T>>(v);
    }

    binding_holder operator()(boost::string_view v) const
    {
        return std::make_unique<binding_text>(v);
    }

    binding_holder operator()(blob_view v) const
    {
        return std::make_unique<binding_blob>(v);
    }
};

inline binding_holder bind_param(const dataset::variant_view* param)
{
    if (param) {
        binding_visitor visitor;
        return boost::apply_visitor(visitor, *param);
    }
    else
        return std::make_unique<binding_stub>();
}

}  // namespace detail
}  // namespace odbc
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_ODBC_DETAIL_BIND_PARAM_HPP
