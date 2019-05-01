// Andrew Naplavkov

#include "utility.h"
#include <QApplication>
#include <QDesktopWidget>
#include <boost/algorithm/string.hpp>

static QString resourceIconPath(const QString& resourceIcon)
{
    return ":/res/" + resourceIcon + ".png";
}

QIcon icon(QStyle::StandardPixmap standard)
{
    return QApplication::desktop()->style()->standardIcon(standard);
}

QIcon icon(const QString& resourceIcon)
{
    return QIcon(resourceIconPath(resourceIcon));
}

QIcon east_icon(const QString& resourceIcon)
{
    return QIcon(QPixmap(resourceIconPath(resourceIcon))
                     .transformed(QTransform().rotate(-90)));
}

QString rich_text(const QString& resourceIcon,
                  const QString& text,
                  Qt::AlignmentFlag icon_alignment)
{
    switch (icon_alignment) {
        case Qt::AlignLeft:
            return QString(R"( <img src="%1" width="16" height="16"/> %2)")
                .arg(resourceIconPath(resourceIcon))
                .arg(text);
        case Qt::AlignRight:
            return QString(R"(%1 <img src="%2" width="16" height="16"/> )")
                .arg(text)
                .arg(resourceIconPath(resourceIcon));
        default:
            return {};
    }
}

QString limited_text(QString text, Qt::AlignmentFlag alignment)
{
    constexpr int LIMIT = 40;
    if (text.size() > LIMIT) {
        if (alignment == Qt::AlignLeft)
            text = text.left(LIMIT) + "...";
        else if (alignment == Qt::AlignRight)
            text = "..." + text.right(LIMIT);
    }
    return text;
}

QString trim_right_copy(const QString& text)
{
    return QString::fromStdString(boost::trim_right_copy(text.toStdString()));
}
