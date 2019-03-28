// Andrew Naplavkov

#ifndef BARK_PROJ_STREAM_HPP
#define BARK_PROJ_STREAM_HPP

#include <bark/detail/wkb.hpp>
#include <bark/proj/detail/transform.hpp>
#include <boost/none.hpp>

namespace bark::proj {

class stream {
public:
    stream(projPJ pj_from, projPJ pj_to) : pj_from_(pj_from), pj_to_(pj_to) {}

    void operator()(blob_view data)
    {
        wkb::istream is(data);
        data_ = data;
        coords_begin_ = coords_end();
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
    const projPJ pj_from_;
    const projPJ pj_to_;
    blob_view data_;
    double* coords_begin_ = nullptr;

    double* coords_end() { return (double*)data_.data(); }

    template <class T>
    stream& operator<<(T val)
    {
        flush();
        *(T*)data_.data() = val;
        data_.remove_prefix(sizeof(T));
        coords_begin_ = coords_end();
        return *this;
    }

    stream& operator<<(double val)
    {
        *(double*)data_.data() = val;
        data_.remove_prefix(sizeof(double));
        return *this;
    }

    void flush()
    {
        if (coords_begin_ == coords_end())
            return;
        transform(pj_from_, pj_to_, coords_begin_, coords_end());
        coords_begin_ = coords_end();
    }
};

}  // namespace bark::proj

#endif  // BARK_PROJ_STREAM_HPP
