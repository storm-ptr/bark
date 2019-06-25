// Andrew Naplavkov

#ifndef BARK_DB_SQL_BUILDER_HPP
#define BARK_DB_SQL_BUILDER_HPP

#include <bark/db/qualified_name.hpp>
#include <bark/db/variant.hpp>
#include <functional>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace bark::db {

/// SQL dialect settings
struct sql_syntax {
    /// An SQL identifier that does not comply with the rules
    /// for the format of regular identifiers must always be delimited.
    std::function<std::string(const std::string& id)>

        delimited_identifier = [](const auto& id) { return '"' + id + '"'; };

    /// A parameter marker is a place holder in an SQL statement whose value is
    /// obtained during statement execution. Empty marker means embedding the
    /// parameters in the SQL text (with quotations if needed).
    std::function<std::string(size_t order)>

        parameter_marker = [](auto) { return "?"; };
};

/// SQL manipulator
template <class T>
struct param {
    const T& val;
};

template <class T>
param(T)->param<T>;

/// Provides a convenient interface to creating and running database queries.

/// @code
/// sql_builder bld = builder(cmd);
/// bld << "SELECT * FROM " << id("sqlite_master")
///     << " WHERE " << id("type") << " = " << param{"view"};
/// cmd.exec(bld);
/// @endcode
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
        auto res = std::vector<variant_t>{};
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
        sql_ << list{name, ".", syntax_.delimited_identifier};
        return *this;
    }

    template <class T>
    sql_builder& operator<<(const param<T>& manip)
    {
        if (syntax_.parameter_marker) {
            sql_ << syntax_.parameter_marker(param_counter_++);
            params_ << manip.val;
        }
        else
            embed_param(manip.val);
        return *this;
    }

    sql_builder& operator<<(const param<qualified_name>& manip)
    {
        return *this << param{(sql_builder{syntax_} << manip.val).sql()};
    }

    template <class T>
    if_arithmetic_t<T, sql_builder&> operator<<(const T& val)
    {
        sql_ << val;
        return *this;
    }

private:
    sql_syntax syntax_;
    std::ostringstream sql_;
    variant_ostream params_;
    size_t param_counter_ = 0;

    void embed_param(const variant_t& var)
    {
        std::visit(
            overloaded{[&](std::monostate) { sql_ << "NULL"; },
                       [&](auto val) { sql_ << val; },
                       [&](std::string_view val) { sql_ << "'" << val << "'"; },
                       [&](blob_view val) { sql_ << "X'" << hex{val} << "'"; }},
            var);
    }

    template <class T>
    if_arithmetic_t<T> embed_param(T val)
    {
        sql_ << val;
    }
};

}  // namespace bark::db

#endif  // BARK_DB_SQL_BUILDER_HPP
