// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_TREE_HPP
#define BARK_QT_DETAIL_TREE_HPP

#include <bark/qt/common.hpp>
#include <boost/blank.hpp>
#include <boost/variant.hpp>
#include <memory>
#include <vector>

namespace bark {
namespace qt {
namespace detail {

using node = boost::variant<boost::blank, link, layer_def>;

struct tree : std::enable_shared_from_this<tree> {
    tree* parent = nullptr;
    std::vector<std::shared_ptr<tree>> children;
    node data;
};

}  // namespace detail
}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_TREE_HPP
