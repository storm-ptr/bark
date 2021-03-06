// Andrew Naplavkov

#ifndef BARK_TEST_SIMPLIFY_GEOMETRY_HPP
#define BARK_TEST_SIMPLIFY_GEOMETRY_HPP

#include <bark/detail/wkb.hpp>
#include <boost/none.hpp>

/// WKB visitor
class simplify_geometry {
public:
    void operator()(bark::blob_view data)
    {
        bark::wkb::istream is(data);
        data_ = data;
        bark::wkb::geometry::accept(is, *this);
    }

    /// reduce precision
    boost::none_t operator()(double x, double y)
    {
        *this << (double)(float)x << (double)(float)y;
        return boost::none;
    }

    template <class T>
    boost::none_t operator()(uint32_t count, T)
    {
        *this << count;
        return boost::none;
    }

    template <class T>
    void operator()(boost::none_t&, const boost::none_t&, T)
    {
    }

    template <class T, uint32_t Code>
    void operator()(bark::wkb::tagged<T, Code>)
    {
        *this << bark::wkb::HostEndian << Code;
    }

    template <class T>
    boost::none_t operator()(const boost::none_t&, T)
    {
        return boost::none;
    }

private:
    bark::blob_view data_;

    template <class T>
    simplify_geometry& operator<<(T val)
    {
        *(T*)data_.data() = val;
        data_.remove_prefix(sizeof(T));
        return *this;
    }
};

#endif  // BARK_TEST_SIMPLIFY_GEOMETRY_HPP
