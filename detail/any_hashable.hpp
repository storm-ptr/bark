// Andrew Naplavkov

#ifndef BARK_ANY_HASHABLE_HPP
#define BARK_ANY_HASHABLE_HPP

#include <any>
#include <boost/functional/hash.hpp>
#include <type_traits>

namespace bark {

class any_hashable {
public:
    any_hashable() = delete;

    template <class T,
              std::enable_if_t<!std::is_same_v<any_hashable, std::decay_t<T>>,
                               void*> = nullptr>
    explicit any_hashable(T&& val)
        : any_{std::forward<T>(val)}
        , hash_{[](const std::any& that) {
            using boost::hash_value;
            return hash_value(std::any_cast<const T&>(that));
        }}
        , equal_{[](const std::any& lhs, const std::any& rhs) {
            return std::any_cast<const T&>(lhs) == std::any_cast<const T&>(rhs);
        }}
    {
    }

    friend size_t hash_value(const any_hashable& that)
    {
        size_t res = 0;
        boost::hash_combine(res, that.any_.type().hash_code());
        boost::hash_combine(res, that.hash_(that.any_));
        return res;
    }

    friend bool operator==(const any_hashable& lhs, const any_hashable& rhs)
    {
        return lhs.any_.type() == rhs.any_.type() &&
               lhs.equal_(lhs.any_, rhs.any_);
    }

private:
    std::any any_;
    size_t (*hash_)(const std::any&);
    bool (*equal_)(const std::any&, const std::any&);
};

}  // namespace bark

#endif  // BARK_ANY_HASHABLE_HPP
