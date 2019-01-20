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
template <class Key, class Mapped>
class linked_hash_map : private boost::noncopyable {
public:
    using value_type = std::pair<const Key, Mapped>;
    using sequence_type = std::list<value_type>;
    using iterator = typename sequence_type::iterator;
    using index_type = std::unordered_map<Key, iterator, boost::hash<Key>>;

    void clear()
    {
        seq_.clear();
        idx_.clear();
    }

    iterator find(const Key& key)
    {
        auto idx_pos = idx_.find(key);
        return idx_pos == idx_.end() ? seq_.end() : idx_pos->second;
    }

    void move(iterator from, iterator to) { seq_.splice(to, seq_, from); }

    iterator erase(iterator pos)
    {
        idx_.erase(pos->first);
        return seq_.erase(pos);  // no-throw guarantee
    }

    std::pair<iterator, bool> insert(iterator pos, const value_type& val)
    {
        auto old_pos = find(val.first);
        if (old_pos != seq_.cend())
            return {old_pos, false};
        auto new_pos = seq_.insert(pos, val);
        try {
            idx_[val.first] = new_pos;
            return {new_pos, true};
        }
        catch (...) {
            seq_.erase(new_pos);  // no-throw guarantee
            throw;
        }
    }

    bool empty() const { return seq_.empty(); }
    size_t size() const { return seq_.size(); }
    iterator begin() { return seq_.begin(); }
    iterator end() { return seq_.end(); }
    value_type& front() { return seq_.front(); }
    value_type& back() { return seq_.back(); }
    void pop_front() { erase(seq_.begin()); }
    void pop_back() { erase(std::prev(seq_.end())); }
    Mapped& operator[](const Key& key) { return find(key)->second; }

private:
    sequence_type seq_;
    index_type idx_;
};

}  // namespace bark

#endif  // BARK_LINKED_HASH_MAP_HPP
