// Andrew Naplavkov

#ifndef BARK_DB_SQL_BUILDER_HPP
#define BARK_DB_SQL_BUILDER_HPP

#include <bark/db/qualified_name.hpp>
#include <bark/db/rowset_ops.hpp>
#include <functional>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace bark::db {

/**
 * An identifier that does not comply with the rules for the format of
 * regular identifiers must always be delimited.
 */
using identifier_delimiter =
    std::function<void(std::ostream& os, std::string_view id)>;

/**
 * A parameter marker is a place holder in an SQL statement whose value is
 * obtained during statement execution. Empty marker means embedding the
 * parameters in the SQL text (with quotations if needed).
 */
using parameter_marker =
    std::function<void(std::ostream& os, size_t param_order)>;

struct sql_syntax {
    identifier_delimiter delimiter = [](std::ostream& os, std::string_view id) {
        os << '"' << id << '"';
    };

    parameter_marker marker = [](std::ostream& os, size_t) { os << "?"; };
};

inline sql_syntax embeded_params(sql_syntax syntax)
{
    syntax.marker = parameter_marker{};
    return syntax;
}

template <class T>
struct param {
    const T& data;
};

template <class T>
param(T)->param<T>;

/**
 * Allows to create database independent SQL queries
 * @code{.cpp}
 * sql_builder bld = builder(cmd);
 * bld << "SELECT * FROM " << id("sqlite_master")
 *     << " WHERE " << id("type") << " = " << param{"view"};
 * cmd.exec(bld);
 * @endcode
 */
class sql_builder {
public:
    explicit sql_builder(sql_syntax syntax) : syntax_{std::move(syntax)}
    {
        sql_.imbue(std::locale::classic());
        sql_.precision(std::numeric_limits<double>::max_digits10);
    }

    auto sql() const { return sql_.str(); }

    auto params() const
    {
        auto res = row_t{};
        for (auto is = variant_istream{params_.data}; !is.data.empty();)
            res.push_back(read(is));
        return res;
    }

    sql_builder& operator<<(std::string_view sql)
    {
        sql_ << sql;
        return *this;
    }

    sql_builder& operator<<(const qualified_name& name)
    {
        sql_ << list{name, ".", [&](std::string_view id) {
                         return delimited{id, syntax_.delimiter};
                     }};
        return *this;
    }

    template <class T>
    sql_builder& operator<<(const param<T>& manip)
    {
        if (syntax_.marker) {
            syntax_.marker(sql_, param_counter_++);
            params_ << manip.data;
        }
        else
            embed_param(manip.data);
        return *this;
    }

    sql_builder& operator<<(const param<qualified_name>& manip)
    {
        return *this << param{(sql_builder{syntax_} << manip.data).sql()};
    }

    template <class T>
    if_arithmetic_t<T, sql_builder&> operator<<(const T& data)
    {
        sql_ << data;
        return *this;
    }

private:
    const sql_syntax syntax_;
    std::ostringstream sql_;
    variant_ostream params_;
    size_t param_counter_ = 0;

    struct delimited {
        std::string_view id;
        const identifier_delimiter& delimiter;

        friend std::ostream& operator<<(std::ostream& os, const delimited& that)
        {
            that.delimiter(os, that.id);
            return os;
        }
    };

    void embed_param(const variant_t& v)
    {
        std::visit(
            overloaded{[&](std::monostate) { sql_ << "NULL"; },
                       [&](auto v) { sql_ << v; },
                       [&](std::string_view v) { sql_ << "'" << v << "'"; },
                       [&](blob_view v) { sql_ << "X'" << hex{v} << "'"; }},
            v);
    }

    template <class T>
    if_arithmetic_t<T> embed_param(T v)
    {
        sql_ << v;
    }
};

}  // namespace bark::db

#endif  // BARK_DB_SQL_BUILDER_HPP
