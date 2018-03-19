// Andrew Naplavkov

#ifndef BARK_QT_COMMON_HPP
#define BARK_QT_COMMON_HPP

#include <QBrush>
#include <QPen>
#include <QPointF>
#include <QSize>
#include <QUrl>
#include <Qt>
#include <bark/db/fwd.hpp>
#include <bark/db/qualified_name.hpp>
#include <string>

namespace bark {
namespace qt {

struct link {
    QUrl uri;
    db::provider_ptr provider;
    bool queryable = false;
};

struct layer_def {
    db::qualified_name name;
    Qt::CheckState state = Qt::Unchecked;
    QPen pen;
    QBrush brush;
};

struct layer : link, layer_def {
    layer() = default;

    layer(link lnk, layer_def lr)
        : link(std::move(lnk)), layer_def(std::move(lr))
    {
    }
};

/**
 * An abstraction of a two-dimensional space that can be drawn on.
 * Its origin is located at the top-left corner.
 * X increases to the right and Y increases downwards. The unit is one pixel.
 */
struct frame {
    QSize size;
    std::string projection;
    QPointF center;
    qreal scale = 1.;
};

}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_COMMON_HPP
