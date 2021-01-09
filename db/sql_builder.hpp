// Andrew Naplavkov

#ifndef BARK_DB_SQL_BUILDER_HPP
#define BARK_DB_SQL_BUILDER_HPP

#include <bark/db/qualified_name.hpp>
#include <bark/db/variant.hpp>
#include <limits>
#include <locale>
#include <string>
#include <vector>

namespace bark::db {

/// An SQL identifier that does not comply with the rules for the format of
/// regular identifiers must always be delimited.
using sql_quoted_identifier = std::function<std::string(std::string_view id)>;

/// A parameter marker is a place holder in an SQL statement whose value is
/// obtained during statement execution. Empty marker means embedding the
/// parameters in the SQL text (with quotations if needed).
using sql_parameter_marker = std::function<std::string(size_t number)>;

/// SQL manipulator
template <class T>
struct param {
    const T& val;
};

template <class T>
param(T) -> param<T>;

/// Convenient interface for preparing database queries.

/// @code
/// sql_builder bld = builder(cmd);
/// bld << "SELECT * FROM " << id("sqlite_master")
///     << " WHERE " << id("type") << " = " << param{"view"};
/// cmd.exec(bld);
/// @endcode
class sql_builder {
public:
    sql_builder(sql_quoted_identifier quoted_identifier,
                sql_parameter_marker parameter_marker)
        : quoted_identifier_{std::move(quoted_identifier)}
        , parameter_marker_{std::move(parameter_marker)}
        , params_number_{0}
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
        sql_ << list{name, ".", quoted_identifier_};
        return *this;
    }

    template <class T>
    sql_builder& operator<<(const param<T>& manip)
    {
        if (parameter_marker_) {
            sql_ << parameter_marker_(params_number_++);
            params_ << manip.val;
        }
        else
            embed_param(manip.val);
        return *this;
    }

    sql_builder& operator<<(const param<qualified_name>& manip)
    {
        auto bld = sql_builder{quoted_identifier_, parameter_marker_};
        bld << manip.val;
        return *this << param{bld.sql()};
    }

    template <class T>
    if_arithmetic_t<T, sql_builder&> operator<<(const T& val)
    {
        sql_ << val;
        return *this;
    }

private:
    sql_quoted_identifier quoted_identifier_;
    sql_parameter_marker parameter_marker_;
    std::ostringstream sql_;
    variant_ostream params_;
    size_t params_number_;

    void embed_param(const variant_t& var)
    {
        std::visit(
            overloaded{[&](std::monostate) { sql_ << "NULL"; },
                       [&](auto v) { sql_ << v; },
                       [&](std::string_view v) { sql_ << "'" << v << "'"; },
                       [&](blob_view v) { sql_ << "X'" << hex{v} << "'"; }},
            var);
    }

    template <class T>
    if_arithmetic_t<T> embed_param(T val)
    {
        sql_ << val;
    }
};

/// Returns a well-known binary representation of a geometry column
using sql_decoder = std::function<void(sql_builder&, std::string_view column)>;

/// Creates a geometry instance from a well-known binary representation
using sql_encoder = std::function<void(sql_builder&, variant_t parameter)>;

}  // namespace bark::db

#endif  // BARK_DB_SQL_BUILDER_HPP
