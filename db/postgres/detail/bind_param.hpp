// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_DETAIL_BIND_PARAM_HPP
#define BARK_DB_POSTGRES_DETAIL_BIND_PARAM_HPP

#include <bark/dataset/variant_view.hpp>
#include <bark/db/postgres/detail/common.hpp>
#include <boost/detail/endian.hpp>
#include <boost/variant/static_visitor.hpp>

namespace bark {
namespace db {
namespace postgres {
namespace detail {

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

template <typename T>
class param_val : public param {
    T val_;

public:
    explicit param_val(T val)
    {
#if defined BOOST_LITTLE_ENDIAN
        val_ = reversed(val);
#elif defined BOOST_BIG_ENDIAN
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

template <typename T, int Format>
class param_arr : public param {
    T arr_;

public:
    explicit param_arr(T arr) : arr_(arr) {}
    Oid type() override { return code_of<T>(); }
    const char* value() override { return (const char*)arr_.data(); }
    int length() override { return int(arr_.size()); }
    int format() override { return Format; }
};

struct bind_param_visitor : boost::static_visitor<param*> {
    param* operator()(boost::blank) const { return new param_null{}; }

    template <typename T>
    param* operator()(std::reference_wrapper<const T> v) const
    {
        return new param_val<T>{v.get()};
    }

    param* operator()(string_view v) const
    {
        return new param_arr<string_view, PGRES_FORMAT_TEXT>{v};
    }

    param* operator()(blob_view v) const
    {
        return new param_arr<blob_view, PGRES_FORMAT_BINARY>{v};
    }
};

inline param_holder bind_param(dataset::variant_view param)
{
    bind_param_visitor visitor;
    return param_holder{boost::apply_visitor(visitor, param)};
}

}  // namespace detail
}  // namespace postgres
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_POSTGRES_DETAIL_BIND_PARAM_HPP
