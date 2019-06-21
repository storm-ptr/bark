// Andrew Naplavkov

#ifndef BARK_DB_QUALIFIED_NAME_HPP
#define BARK_DB_QUALIFIED_NAME_HPP

#include <bark/detail/utility.hpp>
#include <boost/functional/hash.hpp>
#include <ostream>
#include <string>
#include <vector>

namespace bark::db {

/// An unambiguous name.

/// SQL permits names that consist of a single identifier or multiple
/// identifiers. When one named item belongs to another named item, you can (and
/// sometimes must) qualify the name of the "child" item with the name of the
/// "parent" item, using dot notation. For example:
/// @code<schema>.<table>@endcode or @code<table>.<column>@endcode
struct qualified_name : std::vector<std::string> {
    /// @param pos is 0-based, and accepts negative indices
    /// for indexing from the end.
    const std::string& at(int pos) const
    {
        static const std::string Empty{};
        if (pos < 0)
            pos += (int)size();
        return pos < 0 || pos >= (int)size() ? Empty : (*this)[pos];
    }
};

inline std::ostream& operator<<(std::ostream& os, const qualified_name& name)
{
    return os << list{name, "."};
}

inline size_t hash_value(const qualified_name& name)
{
    return boost::hash_range(name.begin(), name.end());
}

inline qualified_name qualifier(qualified_name name)
{
    if (!name.empty())
        name.pop_back();
    return name;
}

template <class... Ts>
auto id(std::string_view parent, Ts&&... children)
{
    qualified_name res;
    if (!parent.empty())
        res.emplace_back(parent);
    (res.emplace_back(std::forward<Ts>(children)), ...);
    return res;
}

inline auto id(qualified_name parent, std::string_view child)
{
    parent.emplace_back(child);
    return parent;
}

}  // namespace bark::db

#endif  // BARK_DB_QUALIFIED_NAME_HPP
