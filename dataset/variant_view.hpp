// Andrew Naplavkov

#ifndef BARK_DATASET_VARIANT_VIEW_HPP
#define BARK_DATASET_VARIANT_VIEW_HPP

#include <bark/common.hpp>
#include <boost/blank.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <functional>

namespace bark {
namespace dataset {

using int64_view = std::reference_wrapper<const int64_t>;
using double_view = std::reference_wrapper<const double>;

using variant_view = boost::
    variant<boost::blank, int64_view, double_view, string_view, blob_view>;

inline bool is_null(const variant_view& v)
{
    return v.which() == which<variant_view, boost::blank>();
}

inline bool test(const variant_view& v)
{
    return !is_null(v) && !!boost::lexical_cast<int64_t>(v);
}

}  // namespace dataset
}  // namespace bark

#endif  // BARK_DATASET_VARIANT_VIEW_HPP
