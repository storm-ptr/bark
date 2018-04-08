// Andrew Naplavkov

#ifndef BARK_DATASET_ROWSET_HPP
#define BARK_DATASET_ROWSET_HPP

#include <bark/dataset/detail/iterator.hpp>
#include <bark/dataset/ostream.hpp>
#include <string>
#include <vector>

namespace bark {
namespace dataset {

inline auto as_vector(blob_view buf)
{
    using iterator = detail::iterator<variant_view>;
    iterator begin{buf.data()};
    iterator end{buf.data() + buf.size()};
    return std::vector<variant_view>(begin, end);
}

class rowset {
public:
    using tuple = std::vector<variant_view>;
    using iterator = detail::iterator<tuple>;

    rowset(std::vector<std::string> cols, blob_t buf)
        : cols_{std::move(cols)}, buf_{std::move(buf)}
    {
    }

    const auto& columns() const { return cols_; }
    bool empty() const { return buf_.empty(); }
    iterator begin() const { return {buf_.data(), cols_.size()}; }
    iterator end() const { return {buf_.data() + buf_.size(), cols_.size()}; }

    friend bool operator==(const rowset& lhs, const rowset& rhs)
    {
        return std::make_tuple(lhs.cols_.size(), blob_view{lhs.buf_}) ==
               std::make_tuple(rhs.cols_.size(), blob_view{rhs.buf_});
    }

private:
    std::vector<std::string> cols_;
    blob_t buf_;
};

inline auto as_vector(const rowset& rows)
{
    return std::vector<rowset::tuple>(rows.begin(), rows.end());
}

}  // namespace dataset
}  // namespace bark

#endif  // BARK_DATASET_ROWSET_HPP
