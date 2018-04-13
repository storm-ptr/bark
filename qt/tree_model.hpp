// Andrew Naplavkov

#ifndef BARK_QT_TREE_MODEL_HPP
#define BARK_QT_TREE_MODEL_HPP

#include <QAbstractItemModel>
#include <QBasicTimer>
#include <QDataStream>
#include <QFuture>
#include <QList>
#include <QModelIndex>
#include <QTimerEvent>
#include <QUrl>
#include <QVariant>
#include <QVector>
#include <bark/qt/common.hpp>
#include <bark/qt/detail/tree.hpp>
#include <boost/optional.hpp>

namespace bark {
namespace qt {

class tree_model : public QAbstractItemModel {
public:
    explicit tree_model(QObject* parent);

    void link_by_uri(QUrl);
    boost::optional<link> get_link(const QModelIndex&) const;
    boost::optional<layer> get_layer(const QModelIndex&) const;
    void set_layer(const QModelIndex&, const layer&);
    void reset();

    friend QDataStream& operator<<(QDataStream& os, const tree_model& that);
    friend QDataStream& operator>>(QDataStream& is, tree_model& that);

    int columnCount(const QModelIndex&) const override;

    QVariant data(const QModelIndex&,
                  int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex&) const override;

    QModelIndex index(int row,
                      int col,
                      const QModelIndex& parent = {}) const override;

    QModelIndex parent(const QModelIndex&) const;

    int rowCount(const QModelIndex& parent = {}) const override;

    bool setData(const QModelIndex&,
                 const QVariant&,
                 int role = Qt::EditRole) override;
    bool removeRows(int position,
                    int rows,
                    const QModelIndex& parent = {}) override;

protected:
    void timerEvent(QTimerEvent*) override;

private:
    std::shared_ptr<detail::tree> root_;
    QList<QFuture<std::shared_ptr<detail::tree>>> future_branches_;
    QBasicTimer timer_;

    detail::tree* to_ptr(const QModelIndex&) const;
};

}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_TREE_MODEL_HPP
