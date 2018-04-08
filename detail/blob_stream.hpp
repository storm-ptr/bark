// Andrew Naplavkov

#ifndef BARK_DETAIL_BLOB_STREAM_HPP
#define BARK_DETAIL_BLOB_STREAM_HPP

#include <bark/common.hpp>
#include <type_traits>

namespace bark {
namespace detail {

class blob_ostream {
    blob_t buf_;

public:
    blob_view buf() const& { return buf_; }
    blob_t buf() && { return std::move(buf_); }

    template <typename T>
    std::enable_if_t<std::is_arithmetic<T>::value && sizeof(T) == 1,
                     blob_ostream&>
    operator<<(T val)
    {
        buf_.push_back((uint8_t)val);
        return *this;
    }

    template <typename T>
    std::enable_if_t<std::is_arithmetic<T>::value && sizeof(T) != 1,
                     blob_ostream&>
    operator<<(T val)
    {
        auto begin = reinterpret_cast<const uint8_t*>(&val);
        auto end = begin + sizeof(T);
        buf_.insert(buf_.end(), begin, end);
        return *this;
    }

    template <typename T, typename value_type = typename T::value_type>
    std::enable_if_t<std::is_arithmetic<value_type>::value, blob_ostream&>
    operator<<(const T& val)
    {
        auto begin = reinterpret_cast<const uint8_t*>(val.data());
        auto end = begin + val.size() * sizeof(value_type);
        buf_.insert(buf_.end(), begin, end);
        return *this;
    }
};

class blob_istream {
    const uint8_t* ptr_;

public:
    explicit blob_istream(const uint8_t* ptr) : ptr_{ptr} {}
    const uint8_t* data() const { return ptr_; };

    template <typename T>
    std::enable_if_t<std::is_arithmetic<T>::value, const T&> read()
    {
        auto res = ptr_;
        ptr_ += sizeof(T);
        return *reinterpret_cast<const T*>(res);
    }

    template <typename T>
    blob_istream& operator>>(T& val)
    {
        val = read<T>();
        return *this;
    }

    template <typename T, typename value_type = typename T::value_type>
    std::enable_if_t<std::is_arithmetic<value_type>::value, T> read(size_t size)
    {
        auto res = ptr_;
        ptr_ += size * sizeof(value_type);
        return T{reinterpret_cast<const value_type*>(res), size};
    }
};

}  // namespace detail
}  // namespace bark

#endif  // BARK_DETAIL_BLOB_STREAM_HPP
