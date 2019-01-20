// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_TREE_HPP
#define BARK_QT_DETAIL_TREE_HPP

#include <bark/qt/common.hpp>
#include <memory>
#include <vector>

namespace bark::qt::detail {

using node = std::variant<std::monostate, link, layer_def>;

struct tree : std::enable_shared_from_this<tree> {
    tree* parent = nullptr;
    std::vector<std::shared_ptr<tree>> children;
    node data;
};

}  // namespace bark::qt::detail

#endif  // BARK_QT_DETAIL_TREE_HPP
