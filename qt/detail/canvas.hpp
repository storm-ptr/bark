// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_CANVAS_HPP
#define BARK_QT_DETAIL_CANVAS_HPP

#include <QImage>
#include <bark/qt/common.hpp>

namespace bark::qt::detail {

struct canvas {
    frame frm;
    QImage img;
};

}  // namespace bark::qt::detail

#endif  // BARK_QT_DETAIL_CANVAS_HPP
