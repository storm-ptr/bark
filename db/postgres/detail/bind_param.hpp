// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_BIND_PARAM_HPP
#define BARK_DB_POSTGRES_BIND_PARAM_HPP

#include <bark/db/postgres/detail/utility.hpp>
#include <bark/db/rowset.hpp>
#include <boost/predef/other/endian.h>

namespace bark::db::postgres {

struct param {
    virtual ~param() = default;
    virtual Oid type() = 0;
    virtual const char* value() = 0;
    virtual int length() = 0;
    virtual int format() = 0;
};

using param_holder = std::unique_ptr<param>;

struct param_null : param {
    Oid type() override { return PGRES_TYPE_TEXT; }
    const char* value() override { return nullptr; }
    int length() override { return 0; }
    int format() override { return PGRES_FORMAT_TEXT; }
};

template <class T>
class param_val : public param {
    T val_;

public:
    explicit param_val(T val)
    {
#if defined BOOST_ENDIAN_LITTLE_BYTE
        val_ = reversed(val);
#elif defined BOOST_ENDIAN_BIG_BYTE
        val_ = val;
#else
#error byte order error
#endif
    }

    Oid type() override { return code_of<T>(); }
    const char* value() override { return (const char*)&val_; }
    int length() override { return sizeof(T); }
    int format() override { return PGRES_FORMAT_BINARY; }
};

template <class T, int Format>
class param_arr : public param {
    T data_;

public:
    explicit param_arr(T data) : data_(data) {}
    Oid type() override { return code_of<T>(); }
    const char* value() override { return (const char*)data_.data(); }
    int length() override { return int(data_.size()); }
    int format() override { return Format; }
};

inline param_holder bind_param(const variant_t& v)
{
    return std::visit(
        overloaded{[](std::monostate) -> param_holder {
                       return std::make_unique<param_null>();
                   },
                   [](auto v) -> param_holder {
                       return std::make_unique<param_val<decltype(v)>>(v);
                   },
                   [](std::string_view v) -> param_holder {
                       return std::make_unique<
                           param_arr<std::string_view, PGRES_FORMAT_TEXT>>(v);
                   },
                   [](blob_view v) -> param_holder {
                       return std::make_unique<
                           param_arr<blob_view, PGRES_FORMAT_BINARY>>(v);
                   }},
        v);
}

}  // namespace bark::db::postgres

#endif  // BARK_DB_POSTGRES_BIND_PARAM_HPP
