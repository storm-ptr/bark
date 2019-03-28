// Andrew Naplavkov

#ifndef BARK_LINKED_HASH_MAP_HPP
#define BARK_LINKED_HASH_MAP_HPP

#include <boost/functional/hash.hpp>
#include <boost/noncopyable.hpp>
#include <list>
#include <unordered_map>

namespace bark {

/**
 * Associative container that maintains insertion order. Complexity of any
 * operation is O(1). Iterators that removed by erase() are invalidated. All
 * other iterators keep their validity.
 */
template <class Key, class T>
class linked_hash_map : boost::noncopyable {
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using container_type = std::list<value_type>;
    using iterator = typename container_type::iterator;
    using index_type = std::unordered_map<Key, iterator, boost::hash<Key>>;

    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }
    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }

    void clear()
    {
        data_.clear();
        idx_.clear();
    }

    iterator find(const Key& key)
    {
        auto it = idx_.find(key);
        return it == idx_.end() ? data_.end() : it->second;
    }

    void move(iterator from, iterator to) { data_.splice(to, data_, from); }

    iterator erase(iterator it)
    {
        idx_.erase(it->first);
        return data_.erase(it);  // no-throw guarantee
    }

    std::pair<iterator, bool> insert(iterator it, value_type val)
    {
        auto res = find(val.first);
        if (res != end())
            return {res, false};
        res = data_.insert(it, std::move(val));
        try {
            idx_[res->first] = res;
            return {res, true};
        }
        catch (...) {
            data_.erase(res);  // no-throw guarantee
            throw;
        }
    }

private:
    container_type data_;
    index_type idx_;
};

}  // namespace bark

#endif  // BARK_LINKED_HASH_MAP_HPP
