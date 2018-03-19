// Andrew Naplavkov

#ifndef BARK_PROJ_DETAIL_STREAM_HPP
#define BARK_PROJ_DETAIL_STREAM_HPP

#include <bark/detail/wkb/visitor.hpp>
#include <bark/proj/detail/transform.hpp>
#include <boost/none.hpp>

namespace bark {
namespace proj {
namespace detail {

class stream {
public:
    stream(projPJ pj_from, projPJ pj_to) : pj_from_(pj_from), pj_to_(pj_to) {}

    uint8_t* operator()(uint8_t* ptr)
    {
        wkb::istream is(ptr);
        ptr_ = ptr;
        coords_begin_ = coords_end();
        wkb::geometry::accept(is, *this);
        flush();
        return const_cast<uint8_t*>(is.data());
    }

    boost::none_t operator()(double x, double y)
    {
        *this << x << y;
        return boost::none;
    }

    template <typename T>
    boost::none_t operator()(uint32_t count, T)
    {
        *this << count;
        return boost::none;
    }

    template <typename T>
    void operator()(boost::none_t&, const boost::none_t&, T)
    {
    }

    template <uint32_t Code, typename T>
    void operator()(wkb::tagged<Code, T>)
    {
        *this << wkb::HostEndian << Code;
    }

    template <typename T>
    boost::none_t operator()(const boost::none_t&, T)
    {
        return boost::none;
    }

private:
    const projPJ pj_from_;
    const projPJ pj_to_;
    uint8_t* ptr_ = nullptr;
    double* coords_begin_ = nullptr;

    double* coords_end() { return reinterpret_cast<double*>(ptr_); }

    template <typename T>
    stream& operator<<(T val)
    {
        flush();
        *reinterpret_cast<T*>(ptr_) = val;
        ptr_ += sizeof(T);
        coords_begin_ = coords_end();
        return *this;
    }

    stream& operator<<(double val)
    {
        *reinterpret_cast<double*>(ptr_) = val;
        ptr_ += sizeof(double);
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

}  // namespace detail
}  // namespace proj
}  // namespace bark

#endif  // BARK_PROJ_DETAIL_STREAM_HPP
