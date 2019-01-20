// Andrew Naplavkov

#ifndef BARK_ANY_HASHABLE_HPP
#define BARK_ANY_HASHABLE_HPP

#include <boost/functional/hash.hpp>
#include <boost/operators.hpp>
#include <memory>
#include <typeinfo>

namespace bark {

/**
 * Polymorphic wrapper of hashable value. std::any analogue
 * @see https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Type_Erasure
 */
class any_hashable : boost::equality_comparable<any_hashable> {
public:
    any_hashable() = delete;
    any_hashable(const any_hashable&) = default;

    template <class T>
    explicit any_hashable(const T& key)
        : ptr_{std::make_shared<wrapper<T>>(key)}
    {
    }

    friend bool operator==(const any_hashable& lhs, const any_hashable& rhs)
    {
        return typeid(*lhs.ptr_) == typeid(*rhs.ptr_) &&
               lhs.ptr_->equal_to(*rhs.ptr_);
    }

    friend size_t hash_value(const any_hashable& that)
    {
        size_t res = 0;
        boost::hash_combine(res, typeid(*that.ptr_).hash_code());
        boost::hash_combine(res, that.ptr_->hash_code());
        return res;
    }

private:
    struct wrapper_base {
        virtual ~wrapper_base() = default;
        virtual bool equal_to(const wrapper_base&) const = 0;
        virtual size_t hash_code() const = 0;
    };

    template <class T>
    class wrapper : public wrapper_base {
        const T key_;

    public:
        explicit wrapper(const T& key) : key_{key} {}

        bool equal_to(const wrapper_base& rhs) const override
        {
            return key_ == dynamic_cast<const wrapper&>(rhs).key_;
        }

        size_t hash_code() const override
        {
            using boost::hash_value;
            return hash_value(key_);
        }
    };

    const std::shared_ptr<const wrapper_base> ptr_;
};

}  // namespace bark

#endif  // BARK_ANY_HASHABLE_HPP
