// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_TREE_IO_HPP
#define BARK_QT_DETAIL_TREE_IO_HPP

#include <QDataStream>
#include <QString>
#include <bark/qt/detail/adapt.hpp>
#include <bark/qt/detail/tree_ops.hpp>
#include <boost/variant/static_visitor.hpp>
#include <unordered_map>

namespace bark {
namespace qt {
namespace detail {

template <typename T>
T read(QDataStream& is)
{
    T res;
    is >> res;
    return res;
}

inline QDataStream& operator<<(QDataStream& os, const node& v)
{
    class visitor : public boost::static_visitor<QDataStream&> {
        QDataStream& os_;

    public:
        visitor(QDataStream& os) : os_(os) {}

        result_type operator()(const boost::blank&) const
        {
            return os_ << which<node, boost::blank>();
        }

        result_type operator()(const link& lnk) const
        {
            return os_ << which<node, link>() << lnk.uri;
        }

        result_type operator()(const layer_def& lr) const
        {
            return os_ << which<node, layer_def>() << adapt(lr.name)
                       << static_cast<quint8>(lr.state) << lr.pen << lr.brush;
        }
    };
    return boost::apply_visitor(visitor(os), v);
}

template <>
inline node read<node>(QDataStream& is)
{
    switch (read<uint8_t>(is)) {
        case which<node, boost::blank>(): {
            return boost::blank{};
        }
        case which<node, link>(): {
            link lnk;
            is >> lnk.uri;
            return lnk;
        }
        case which<node, layer_def>(): {
            layer_def lr;
            lr.name = adapt(read<QStringList>(is));
            lr.state = static_cast<Qt::CheckState>(read<quint8>(is));
            is >> lr.pen >> lr.brush;
            return lr;
        }
        default:
            throw std::runtime_error("invalid node");
    }
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

}  // namespace detail
}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_TREE_IO_HPP
