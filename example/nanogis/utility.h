// Andrew Naplavkov

#ifndef UTILITY_H
#define UTILITY_H

#include <QIcon>
#include <QString>
#include <QStyle>
#include <Qt>
#include <bark/qt/common.hpp>
#include <map>
#include <string>
#include <vector>

using string_map = std::map<std::string, std::string>;

struct layer_columns {
    bark::qt::layer lr;
    std::vector<std::string> cols;
};

QIcon icon(QStyle::StandardPixmap);

QIcon icon(const QString& resourceIcon);

QIcon east_icon(const QString& resourceIcon);

QString rich_text(const QString& resourceIcon,
                  const QString& text,
                  Qt::AlignmentFlag icon_alignment);

QString limited_text(QString text, Qt::AlignmentFlag alignment);

QString trim_right_copy(const QString&);

#endif  // UTILITY_H
