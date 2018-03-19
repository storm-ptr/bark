// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_ADAPT_HPP
#define BARK_QT_DETAIL_ADAPT_HPP

#include <QPointF>
#include <QString>
#include <QStringList>
#include <bark/db/qualified_name.hpp>
#include <bark/geometry/geometry.hpp>
#include <string>

namespace bark {
namespace qt {
namespace detail {

inline QPointF adapt(const geometry::point& point)
{
    return {point.x(), point.y()};
}

inline geometry::point adapt(const QPointF& point)
{
    return {point.x(), point.y()};
}

inline QString adapt(const std::string& str)
{
    return QString::fromStdString(str);
}

inline std::string adapt(const QString& str)
{
    return str.toStdString();
}

inline auto adapt(const db::qualified_name& name)
{
    return back_constructor<QStringList>(
        name, [](auto& item) { return adapt(item); });
}

inline auto adapt(const QStringList& name)
{
    return back_constructor<db::qualified_name>(
        name, [](auto& item) { return adapt(item); });
}

}  // namespace detail

using detail::adapt;

}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_ADAPT_HPP
