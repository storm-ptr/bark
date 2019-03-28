// Andrew Naplavkov

#ifndef BARK_QT_ADAPT_HPP
#define BARK_QT_ADAPT_HPP

#include <QPointF>
#include <QString>
#include <QStringList>
#include <bark/db/qualified_name.hpp>
#include <bark/geometry/geometry.hpp>
#include <string>

namespace bark::qt {

struct adaptor {
    auto operator()(const geometry::point& v) const
    {
        return QPointF{v.x(), v.y()};
    }

    auto operator()(const QPointF& v) const
    {
        return geometry::point{v.x(), v.y()};
    }

    auto operator()(const std::string& v) const
    {
        return QString::fromStdString(v);
    }

    auto operator()(const QString& v) const { return v.toStdString(); }

    auto operator()(const db::qualified_name& v) const
    {
        return as<QStringList>(v, *this);
    }

    auto operator()(const QStringList& v) const
    {
        return as<db::qualified_name>(v, *this);
    }
};

template <class T>
auto adapt(T&& v)
{
    return adaptor{}(std::forward<T>(v));
}

}  // namespace bark::qt

#endif  // BARK_QT_ADAPT_HPP
