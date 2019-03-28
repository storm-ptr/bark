// Andrew Naplavkov

#ifndef BARK_QT_TREE_HPP
#define BARK_QT_TREE_HPP

#include <bark/qt/common.hpp>
#include <memory>
#include <vector>

namespace bark::qt {

using node = std::variant<std::monostate, link, layer_def>;

struct tree : std::enable_shared_from_this<tree> {
    tree* parent = nullptr;
    std::vector<std::shared_ptr<tree>> children;
    node data;
};

}  // namespace bark::qt

#endif  // BARK_QT_TREE_HPP
