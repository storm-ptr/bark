// Andrew Naplavkov

#ifndef BARK_PROJ_STREAM_HPP
#define BARK_PROJ_STREAM_HPP

#include <bark/detail/wkb.hpp>
#include <bark/proj/detail/transform.hpp>
#include <boost/none.hpp>

namespace bark::proj {

/// WKB visitor
class stream {
public:
    stream(projPJ from, projPJ to) : from_(from), to_(to) {}

    void operator()(blob_view wkb)
    {
        wkb::istream is(wkb);
        wkb_ = wkb;
        begin_ = end();
        wkb::geometry::accept(is, *this);
        flush();
    }

    boost::none_t operator()(double x, double y)
    {
        *this << x << y;
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
    void operator()(wkb::tagged<T, Code>)
    {
        *this << wkb::HostEndian << Code;
    }

    template <class T>
    boost::none_t operator()(const boost::none_t&, T)
    {
        return boost::none;
    }

private:
    const projPJ from_;
    const projPJ to_;
    blob_view wkb_;
    double* begin_ = nullptr;

    double* end() { return (double*)wkb_.data(); }

    template <class T>
    stream& operator<<(T v)
    {
        flush();
        *(T*)wkb_.data() = v;
        wkb_.remove_prefix(sizeof(T));
        begin_ = end();
        return *this;
    }

    stream& operator<<(double v)
    {
        *(double*)wkb_.data() = v;
        wkb_.remove_prefix(sizeof(double));
        return *this;
    }

    void flush()
    {
        if (begin_ == end())
            return;
        transform(from_, to_, begin_, end());
        begin_ = end();
    }
};

}  // namespace bark::proj

#endif  // BARK_PROJ_STREAM_HPP
