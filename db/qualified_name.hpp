// Andrew Naplavkov

#ifndef BARK_DB_QUALIFIED_NAME_HPP
#define BARK_DB_QUALIFIED_NAME_HPP

#include <bark/common.hpp>
#include <boost/functional/hash.hpp>
#include <ostream>
#include <string>
#include <vector>

namespace bark {
namespace db {

/**
 * SQL permits names that consist of a single identifier or multiple
 * identifiers. When one named item belongs to another named item, you can (and
 * sometimes must) qualify the name of the "child" item with the name of the
 * "parent" item, using dot notation. For example: <db>.<schema>.<table>
 * or <table>.<column>
 */
struct qualified_name : std::vector<std::string> {
};

inline const std::string& reverse_at(const qualified_name& name, size_t pos)
{
    static const std::string Empty{};
    return pos < name.size() ? name[name.size() - pos - 1] : Empty;
}

inline qualified_name qualifier(const qualified_name& name)
{
    qualified_name res{name};
    if (!res.empty())
        res.pop_back();
    return res;
}

inline std::ostream& operator<<(std::ostream& os, const qualified_name& name)
{
    return os << list(name, ".");
}

inline size_t hash_value(const qualified_name& name)
{
    return boost::hash_range(name.begin(), name.end());
}

inline qualified_name id()
{
    return {};
}

template <typename... T>
auto id(string_view parent, T&&... children)
{
    auto res = id(std::forward<T>(children)...);
    if (!parent.empty())
        res.insert(res.begin(), parent.to_string());
    return res;
}

inline auto id(qualified_name parent, string_view child)
{
    parent.push_back(child.to_string());
    return parent;
}

}  // namespace db
}  // namespace bark

#endif  // BARK_DB_QUALIFIED_NAME_HPP
