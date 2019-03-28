// Andrew Naplavkov

#ifndef BARK_ANY_HASHABLE_HPP
#define BARK_ANY_HASHABLE_HPP

#include <boost/functional/hash.hpp>
#include <memory>
#include <typeinfo>

namespace bark {

/**
 * Polymorphic wrapper of hashable value. std::any analogue.
 * @see https://en.wikibooks.org/wiki/More_C++_Idioms/Type_Erasure
 */
class any_hashable {
public:
    any_hashable() = delete;

    template <class T>
    explicit any_hashable(const T& val) : ptr_{std::make_shared<model<T>>(val)}
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
    struct concept
    {
        virtual ~concept() = default;
        virtual bool equal_to(const concept&) const = 0;
        virtual size_t hash_code() const = 0;
    };

    template <class T>
    class model : public concept {
        T val_;

    public:
        explicit model(const T& val) : val_{val} {}

        bool equal_to(const concept& that) const override
        {
            return this->val_ == dynamic_cast<const model&>(that).val_;
        }

        size_t hash_code() const override
        {
            using boost::hash_value;
            return hash_value(val_);
        }
    };

    std::shared_ptr<concept> ptr_;
};

}  // namespace bark

#endif  // BARK_ANY_HASHABLE_HPP
