// Andrew Naplavkov

#ifndef BARK_TEST_WKB_UNIFIER_HPP
#define BARK_TEST_WKB_UNIFIER_HPP

#include <bark/detail/wkb/visitor.hpp>
#include <boost/none.hpp>

class wkb_unifier {
public:
    uint8_t* operator()(uint8_t* ptr)
    {
        bark::wkb::istream is(ptr);
        ptr_ = ptr;
        bark::wkb::geometry::accept(is, *this);
        return const_cast<uint8_t*>(is.data());
    }

    boost::none_t operator()(double x, double y)
    {
        *this << (double)(float)x << (double)(float)y;
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
    void operator()(bark::wkb::tagged<Code, T>)
    {
        *this << bark::wkb::HostEndian << Code;
    }

    template <typename T>
    boost::none_t operator()(const boost::none_t&, T)
    {
        return boost::none;
    }

private:
    uint8_t* ptr_ = nullptr;

    template <typename T>
    wkb_unifier& operator<<(T val)
    {
        *reinterpret_cast<T*>(ptr_) = val;
        ptr_ += sizeof(T);
        return *this;
    }
};

inline void unify(bark::dataset::rowset& rows, size_t col)
{
    wkb_unifier unifier;
    for (auto& row : rows)
        unifier(
            const_cast<uint8_t*>(boost::get<bark::blob_view>(row[col]).data()));
}

#endif  // BARK_TEST_WKB_UNIFIER_HPP
