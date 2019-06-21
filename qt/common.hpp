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

namespace bark::qt {

/// Spatial data source
struct link {
    QUrl uri;
    db::provider_ptr provider;
    bool queryable = false;
};

/// Describes how to display spatial data set
struct layer_def {
    db::qualified_name name;
    Qt::CheckState state = Qt::Unchecked;
    QPen pen;
    QBrush brush;
};

/// Map layer is used to display and work with a data set
struct layer : link, layer_def {
    layer() = default;

    layer(link lnk, layer_def lr)
        : link(std::move(lnk)), layer_def(std::move(lr))
    {
    }
};

/// A two-dimensional space that can be drawn on.

/// Its origin is located at the top-left corner.
/// X increases to the right and Y increases downwards.
/// The unit is one pixel.
struct frame {
    QSize size;
    QPointF center;
    qreal scale = 1.;
    std::string projection;
};

}  // namespace bark::qt

#endif  // BARK_QT_COMMON_HPP
