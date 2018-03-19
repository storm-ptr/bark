// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_CANVAS_HPP
#define BARK_QT_DETAIL_CANVAS_HPP

#include <QImage>
#include <bark/qt/common.hpp>

namespace bark {
namespace qt {
namespace detail {

struct canvas {
    frame frm;
    QImage img;
};

}  // namespace detail
}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_CANVAS_HPP
