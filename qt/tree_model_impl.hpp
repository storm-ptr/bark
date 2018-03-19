// Andrew Naplavkov
// include in *.cpp files only (Qt MOC workaround)

#ifndef BARK_QT_TREE_MODEL_IMPL_HPP
#define BARK_QT_TREE_MODEL_IMPL_HPP

#include <QtConcurrent/QtConcurrentRun>
#include <bark/common.hpp>
#include <bark/qt/detail/tree_io.hpp>
#include <bark/qt/detail/tree_ops.hpp>
#include <bark/qt/tree_model.hpp>
#include <exception>

namespace bark {
namespace qt {

inline tree_model::tree_model(QObject* parent) : QAbstractItemModel(parent)
{
    using namespace std::chrono;
    root_ = std::make_shared<detail::tree>();
    reset();
    timer_.start(duration_cast<milliseconds>(UiTimeout).count(), this);
}

inline detail::tree* tree_model::to_ptr(const QModelIndex& idx) const
{
    auto ptr = idx.isValid() ? reinterpret_cast<detail::tree*>(idx.internalId())
                             : nullptr;
    return ptr ? ptr : root_.get();
}

inline void tree_model::link_by_uri(QUrl uri)
{
    future_branches_.push_back(
        QtConcurrent::run([=] { return detail::dir(uri); }));
}

inline void tree_model::reset()
{
    future_branches_.clear();
    if (!root_->children.empty())
        removeRows(0, (int)root_->children.size());
    link_by_uri(QString::fromStdString("slippy://"));
}

inline void tree_model::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != timer_.timerId())
        return QAbstractItemModel::timerEvent(event);

    auto it = future_branches_.begin();
    while (it != future_branches_.end()) {
        if (it->resultCount() && it->isResultReadyAt(0)) {
            try {
                auto branch = it->resultAt(0);
                auto rng = binary_search(root_.get(), branch.get());
                auto row = rng.first - root_->children.begin();
                if (rng.first != rng.second) {
                    copy_children_data(rng.first->get(), branch.get());
                    removeRows(row, rng.second - rng.first);
                }

                beginInsertRows({}, row, row);
                branch->parent = root_.get();
                root_->children.insert(root_->children.begin() + row, branch);
                endInsertRows();
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            it = future_branches_.erase(it);
        }
        else {
            ++it;
        }
    }
}

inline boost::optional<link> tree_model::get_link(const QModelIndex& idx) const
{
    return detail::get_link(to_ptr(idx));
}

inline boost::optional<layer> tree_model::get_layer(
    const QModelIndex& idx) const
{
    return detail::get_layer(to_ptr(idx));
}

inline void tree_model::set_layer(const QModelIndex& idx, const layer& lr)
{
    detail::set_layer(to_ptr(idx), lr);
    dataChanged(idx, idx);
}

inline int tree_model::columnCount(const QModelIndex&) const
{
    return 1;
}

inline QVariant tree_model::data(const QModelIndex& idx, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            return to_string(to_ptr(idx));
        case Qt::CheckStateRole: {
            auto opt = state(to_ptr(idx));
            if (opt)
                return *opt;
        } break;
    }
    return {};
}

inline Qt::ItemFlags tree_model::flags(const QModelIndex& idx) const
{
    auto res = Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    if (get_layer(idx))
        res |= Qt::ItemIsUserCheckable;
    return res;
}

inline QModelIndex tree_model::index(int row,
                                     int,
                                     const QModelIndex& parent) const
{
    auto ptr = to_ptr(parent);
    if (row < 0 || row >= (int)ptr->children.size())
        return {};
    return createIndex(row, 0, to_ptr(parent)->children[row].get());
}

inline QModelIndex tree_model::parent(const QModelIndex& idx) const
{
    auto ptr = to_ptr(idx);
    if (!ptr->parent)
        return {};
    auto rng = binary_search(ptr->parent, ptr);
    if (rng.first == rng.second)
        return {};
    auto row = rng.first - ptr->parent->children.begin();
    return createIndex(row, 0, ptr->parent);
}

inline int tree_model::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(to_ptr(parent)->children.size());
}

inline bool tree_model::setData(const QModelIndex& idx,
                                const QVariant&,
                                int role)
{
    if (role != Qt::CheckStateRole || !toggle(to_ptr(idx)))
        return false;
    dataChanged(idx, idx);
    return true;
}

inline bool tree_model::removeRows(int position,
                                   int rows,
                                   const QModelIndex& parent)
{
    if (rows > 0) {
        beginRemoveRows(parent, position, position + rows - 1);
        auto ptr = to_ptr(parent);
        auto begin = ptr->children.begin() + position;
        auto end = begin + rows;
        ptr->children.erase(begin, end);
        endRemoveRows();
    }
    return true;
}

inline QDataStream& operator<<(QDataStream& os, const tree_model& that)
{
    return os << *that.root_;
}

inline QDataStream& operator>>(QDataStream& is, tree_model& that)
{
    that.future_branches_.clear();
    auto root = detail::read<std::shared_ptr<detail::tree>>(is);
    that.removeRows(0, (int)that.root_->children.size());
    that.beginInsertRows({}, 0, (int)root->children.size());
    that.root_ = root;
    that.endInsertRows();
    for (auto& child : that.root_->children)
        that.link_by_uri(boost::get<link>(child->data).uri);
    return is;
}

}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_TREE_MODEL_IMPL_HPP
