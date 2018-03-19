// Andrew Naplavkov

#ifndef BARK_DB_SQL_BUILDER_HPP
#define BARK_DB_SQL_BUILDER_HPP

#include <bark/dataset/ostream.hpp>
#include <bark/db/qualified_name.hpp>
#include <boost/variant/static_visitor.hpp>
#include <functional>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace bark {
namespace db {

/**
 * An identifier that does not comply with the rules for the format of
 * regular identifiers must always be delimited.
 */
using identifier_delimiter =
    std::function<void(std::ostream& os, boost::string_view id)>;

/**
 * A parameter marker is a place holder in an SQL statement whose value is
 * obtained during statement execution. Empty marker means embedding the
 * parameters in the SQL text (with quotations if needed).
 */
using parameter_marker =
    std::function<void(std::ostream& os, size_t param_order)>;

struct sql_syntax {
    identifier_delimiter delimiter =
        [](std::ostream& os, boost::string_view id) { os << '"' << id << '"'; };

    parameter_marker marker = [](std::ostream& os, size_t) { os << "?"; };
};

inline sql_syntax embeded_params(const sql_syntax& syntax)
{
    return {syntax.delimiter, {}};
}

namespace detail {

template <typename T>
struct param_manipulator {
    const T& val;
};

}  // namespace detail

template <typename T>
detail::param_manipulator<T> param(const T& val)
{
    return {val};
}

/**
 * Allows to create database independent SQL queries
 * @code{.cpp}
 * sql_builder bld = builder(cmd);
 * bld << "SELECT * FROM " << id("sqlite_master")
 *     << " WHERE " << id("type") << " = " << param("view");
 * cmd.exec(bld);
 * @endcode
 */
class sql_builder {
public:
    explicit sql_builder(const sql_syntax& syntax) : syntax_{syntax}
    {
        sql_.imbue(std::locale::classic());
        sql_.precision(std::numeric_limits<double>::max_digits10);
    }

    auto sql() const { return sql_.str(); }

    auto params() const { return dataset::as_vector(params_.buf()); }

    sql_builder& operator<<(boost::string_view sql)
    {
        sql_ << sql;
        return *this;
    }

    sql_builder& operator<<(const qualified_name& name)
    {
        sql_ << list(name, ".", [&](auto& item) {
            return delimiter_manipulator{syntax_.delimiter, item};
        });
        return *this;
    }

    template <typename T>
    sql_builder& operator<<(const detail::param_manipulator<T>& manip)
    {
        if (syntax_.marker) {
            syntax_.marker(sql_, param_counter_++);
            params_ << manip.val;
        }
        else
            embed_param(manip.val);
        return *this;
    }

    sql_builder& operator<<(
        const detail::param_manipulator<qualified_name>& manip)
    {
        return *this << param((sql_builder{syntax_} << manip.val).sql());
    }

    template <typename T>
    std::enable_if_t<std::is_arithmetic<T>::value, sql_builder&> operator<<(
        const T& val)
    {
        sql_ << val;
        return *this;
    }

private:
    const sql_syntax syntax_;
    std::ostringstream sql_;
    dataset::ostream params_;
    size_t param_counter_ = 0;

    struct delimiter_manipulator {
        const identifier_delimiter& delimiter;
        boost::string_view id;

        friend std::ostream& operator<<(std::ostream& os,
                                        const delimiter_manipulator& manip)
        {
            manip.delimiter(os, manip.id);
            return os;
        }
    };

    class embedded_param_visitor : public boost::static_visitor<void> {
        std::ostringstream& sql_;

    public:
        explicit embedded_param_visitor(std::ostringstream& sql) : sql_(sql) {}
        void operator()(boost::blank) const { sql_ << "NULL"; }
        void operator()(boost::string_view v) const { sql_ << "'" << v << "'"; }
        void operator()(blob_view v) const { sql_ << "X'" << hex(v) << "'"; }

        template <typename T>
        void operator()(std::reference_wrapper<const T> v) const
        {
            sql_ << v.get();
        }
    };

    void embed_param(const dataset::variant_view& v)
    {
        boost::apply_visitor(embedded_param_visitor{sql_}, v);
    }

    template <typename T>
    std::enable_if_t<std::is_arithmetic<T>::value> embed_param(T v)
    {
        sql_ << v;
    }
};

}  // namespace db
}  // namespace bark

#endif  // BARK_DB_SQL_BUILDER_HPP
