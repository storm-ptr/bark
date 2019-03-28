// Andrew Naplavkov

#ifndef BARK_QT_TREE_IO_HPP
#define BARK_QT_TREE_IO_HPP

#include <QDataStream>
#include <QString>
#include <bark/qt/detail/adapt.hpp>
#include <bark/qt/detail/tree_ops.hpp>
#include <unordered_map>

namespace bark::qt {

template <class T>
T read(QDataStream& is)
{
    T res;
    is >> res;
    return res;
}

inline QDataStream& operator<<(QDataStream& os, const node& v)
{
    os << static_cast<quint8>(v.index());
    std::visit(overloaded{[&](const std::monostate&) {},
                          [&](const link& v) { os << v.uri; },
                          [&](const layer_def& v) {
                              os << adapt(v.name)
                                 << static_cast<quint8>(v.state) << v.pen
                                 << v.brush;
                          }},
               v);
    return os;
}

template <>
inline node read<node>(QDataStream& is)
{
    switch (read<quint8>(is)) {
        case variant_index<node, std::monostate>():
            return {};
        case variant_index<node, link>(): {
            link lnk;
            is >> lnk.uri;
            return lnk;
        }
        case variant_index<node, layer_def>(): {
            layer_def lr;
            lr.name = adapt(read<QStringList>(is));
            lr.state = static_cast<Qt::CheckState>(read<quint8>(is));
            is >> lr.pen >> lr.brush;
            return lr;
        }
    }
    throw std::runtime_error("invalid node");
}

inline QDataStream& operator<<(QDataStream& os, const tree& tr)
{
    os << tr.data << static_cast<quint32>(tr.children.size());
    for (auto& child : tr.children)
        os << *child;
    return os;
}

template <>
inline std::shared_ptr<tree> read<std::shared_ptr<tree>>(QDataStream& is)
{
    auto res = std::make_shared<tree>();
    res->data = read<node>(is);
    res->children.resize(read<quint32>(is));
    for (auto& child : res->children) {
        child = read<std::shared_ptr<tree>>(is);
        child->parent = res.get();
    }
    return res;
}

}  // namespace bark::qt

#endif  // BARK_QT_TREE_IO_HPP
