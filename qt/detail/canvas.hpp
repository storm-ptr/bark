// Andrew Naplavkov

#ifndef BARK_QT_CANVAS_HPP
#define BARK_QT_CANVAS_HPP

#include <QImage>
#include <bark/qt/common.hpp>

namespace bark::qt {

struct canvas {
    frame frm;
    QImage img;
};

}  // namespace bark::qt

#endif  // BARK_QT_CANVAS_HPP
