// Andrew Naplavkov

#ifndef BARK_DATASET_DETAIL_ITERATOR_HPP
#define BARK_DATASET_DETAIL_ITERATOR_HPP

#include <bark/dataset/detail/istream.hpp>
#include <boost/operators.hpp>
#include <cstddef>
#include <cstdint>
#include <iterator>

namespace bark {
namespace dataset {
namespace detail {

template <typename T>
class iterator : public boost::forward_iteratable<iterator<T>, T*> {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    template <typename... Args>
    iterator(const uint8_t* data, Args&&... args)
        : begin_{data}, end_{data}, val_(std::forward<Args>(args)...)
    {
    }

    friend bool operator==(const iterator& lhs, const iterator& rhs)
    {
        return lhs.begin_.data() == rhs.begin_.data();
    }

    iterator& operator++()
    {
        read();
        begin_ = end_;
        return *this;
    }

    const T& operator*() const
    {
        read();
        return val_;
    }

private:
    istream begin_;
    mutable istream end_;
    mutable std::remove_const_t<T> val_;

    void read() const
    {
        if (begin_.data() == end_.data())
            end_ >> val_;
    }
};

}  // namespace detail
}  // namespace dataset
}  // namespace bark

#endif  // BARK_DATASET_DETAIL_ITERATOR_HPP
