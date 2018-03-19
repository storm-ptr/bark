// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_TREE_OPS_HPP
#define BARK_QT_DETAIL_TREE_OPS_HPP

#include <QString>
#include <algorithm>
#include <bark/db/gdal/provider.hpp>
#include <bark/db/mysql/provider.hpp>
#include <bark/db/odbc/provider.hpp>
#include <bark/db/postgres/provider.hpp>
#include <bark/db/slippy/provider.hpp>
#include <bark/db/sqlite/provider.hpp>
#include <bark/qt/detail/adapt.hpp>
#include <bark/qt/detail/tree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/variant/static_visitor.hpp>
#include <stdexcept>

namespace bark {
namespace qt {
namespace detail {

inline auto trimmed_path(const QUrl& uri)
{
    auto res = uri.path();
    res.replace("/", "");
    return res;
}

template <typename Provider, typename... Args>
link make_link(const QUrl& uri, Args&&... args)
{
    auto pvd = std::make_shared<Provider>(std::forward<Args>(args)...);
    return {uri, pvd, db::queryable(*pvd)};
}

template <typename Provider>
link make_file_link(const QUrl& uri)
{
    auto file = uri;
    file.setScheme("file");
    return make_link<Provider>(uri, adapt(file.toLocalFile()));
}

template <typename Provider, int Port>
link make_server_link(const QUrl& uri)
{
    return make_link<Provider>(uri,
                               adapt(uri.host()),
                               uri.port(Port),
                               adapt(trimmed_path(uri)),
                               adapt(uri.userName()),
                               adapt(uri.password()));
}

inline link make_odbc_link(const QUrl& uri)
{
    std::ostringstream os;
    if (!uri.userName().isEmpty())
        os << "UID=" << adapt(uri.userName()) << ";";
    if (!uri.password().isEmpty())
        os << "PWD=" << adapt(uri.password()) << ";";
    if (!uri.host().isEmpty())
        os << "SERVER=" << adapt(uri.host()) << ";"
           << "HOSTNAME=" << adapt(uri.host()) << ";";
    auto port = uri.port(-1);
    if (port > 0)
        os << "PORT=" << port << ";";
    if (!uri.path().isEmpty())
        os << "DATABASE=" << adapt(trimmed_path(uri)) << ";";
    os << adapt(uri.query());
    return make_link<db::odbc::provider>(uri, os.str());
}

inline link make_link(const QUrl& uri)
{
    auto scm = uri.scheme();
    if (scm == "gdal")
        return make_file_link<db::gdal::provider>(uri);
    else if (scm == "mysql")
        return make_server_link<db::mysql::provider, 3306>(uri);
    else if (scm == "odbc")
        return make_odbc_link(uri);
    else if (scm == "postgresql")
        return make_server_link<db::postgres::provider, 5432>(uri);
    else if (scm == "slippy")
        return make_link<db::slippy::provider>(uri);
    else if (scm == "sqlite")
        return make_file_link<db::sqlite::provider>(uri);
    throw std::runtime_error("invalid URI: " + adapt(uri.toString()));
}

inline QString to_string(tree* ptr)
{
    struct visitor : boost::static_visitor<QString> {
        result_type operator()(const boost::blank&) const { return {}; }

        result_type operator()(const link& v) const
        {
            return v.uri.toDisplayString(QUrl::DecodeReserved);
        }

        result_type operator()(const layer_def& v) const
        {
            return adapt(boost::lexical_cast<std::string>(v.name));
        }
    };
    return boost::apply_visitor(visitor{}, ptr->data);
}

inline auto binary_search(tree* parent, tree* child)
{
    return std::equal_range(
        parent->children.begin(),
        parent->children.end(),
        child->shared_from_this(),
        [](const std::shared_ptr<tree>& lhs, const std::shared_ptr<tree>& rhs) {
            return to_string(lhs.get()) < to_string(rhs.get());
        });
}

inline std::shared_ptr<tree> dir(const QUrl& uri)
{
    auto res = std::make_shared<tree>();
    auto lnk = make_link(uri);
    res->data = lnk;
    for (auto& pair : lnk.provider->dir()) {
        layer_def def;
        def.name = pair.first;
        auto child = std::make_shared<tree>();
        child->parent = res.get();
        child->data = def;
        res->children.push_back(child);
    }
    return res;
}

inline void copy_children_data(tree* from, tree* to)
{
    for (auto& child : to->children) {
        auto rng = binary_search(from, child.get());
        if (rng.first != rng.second) {
            child->data = (*rng.first)->data;
        }
    }
}

inline bool is_link(tree* ptr)
{
    return ptr->data.which() == which<node, link>();
}

inline bool is_layer(tree* ptr)
{
    return ptr->data.which() == which<node, layer_def>();
}

inline boost::optional<Qt::CheckState> state(tree* ptr)
{
    if (!is_layer(ptr))
        return {};
    return boost::get<layer_def>(ptr->data).state;
}

inline bool toggle(tree* ptr)
{
    if (!is_layer(ptr))
        return {};
    auto& lr = boost::get<layer_def>(ptr->data);
    switch (lr.state) {
        case Qt::Unchecked:
            lr.state = Qt::Checked;
            return true;
        case Qt::Checked:
            lr.state = Qt::Unchecked;
            return true;
        default:
            return false;
    }
}

inline boost::optional<link> get_link(tree* ptr)
{
    if (!is_link(ptr))
        return {};
    return boost::get<link>(ptr->data);
}

inline boost::optional<layer> get_layer(tree* ptr)
{
    if (!ptr->parent || !is_link(ptr->parent) || !is_layer(ptr))
        return {};
    return layer{boost::get<link>(ptr->parent->data),
                 boost::get<layer_def>(ptr->data)};
}

inline void set_layer(tree* ptr, const layer& lr)
{
    if (!ptr->parent || !is_link(ptr->parent) || !is_layer(ptr))
        return;
    auto& lnk = boost::get<link>(ptr->parent->data);
    auto& def = boost::get<layer_def>(ptr->data);
    if (lnk.uri == lr.uri && def.name == lr.name)
        def = lr;
}

}  // namespace detail
}  // namespace qt
}  // namespace bark

#endif  // BARK_QT_DETAIL_TREE_OPS_HPP
