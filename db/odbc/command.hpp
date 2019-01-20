// Andrew Naplavkov

#ifndef BARK_DB_ODBC_COMMAND_HPP
#define BARK_DB_ODBC_COMMAND_HPP

#include <bark/db/command.hpp>
#include <bark/db/odbc/detail/bind_column.hpp>
#include <bark/db/odbc/detail/bind_param.hpp>
#include <bark/unicode.hpp>

namespace bark::db::odbc {

class command : public db::command {
public:
    explicit command(std::string_view conn_str)
    {
        using namespace std::chrono;

        env_.reset(alloc_handle(detail::env_holder{}, SQL_HANDLE_ENV));
        set_attr(env_, SQL_ATTR_ODBC_VERSION, SQL_OV_ODBC3);

        dbc_.reset(alloc_handle(env_, SQL_HANDLE_DBC));
        set_attr(dbc_,
                 SQL_ATTR_LOGIN_TIMEOUT,
                 duration_cast<seconds>(DbTimeout).count());

        SQLSMALLINT len = 0;
        check(dbc_,
              SQLDriverConnectW(
                  dbc_.get(),
                  0,
                  (SQLWCHAR*)unicode::to_string<SQLWCHAR>(conn_str).c_str(),
                  SQL_NTS,
                  0,
                  0,
                  &len,
                  SQL_DRIVER_NOPROMPT));
    }

    sql_syntax syntax() override
    {
        db::sql_syntax res{};
        res.delimiter = [quote = get_info(dbc_, SQL_IDENTIFIER_QUOTE_CHAR)](
                            std::ostream& os, std::string_view id) {
            os << quote << id << quote;
        };
        return res;
    }

    command& exec(const sql_builder& bld) override
    {
        prepare(bld.sql());
        auto bnds = bind_params(bld.params());
        auto r = SQLExecute(stmt_.get());
        if (SQL_NO_DATA != r)
            check(stmt_, r);
        more_results();
        return *this;
    }

    std::vector<std::string> columns() override
    {
        std::vector<std::string> names;
        cols_.clear();

        for (SQLUSMALLINT i = 1, cols = num_result_cols(); i <= cols; ++i) {
            names.push_back(string_attr(stmt_, i, SQL_DESC_NAME));
            cols_.push_back(bind_column(stmt_, i));
        }
        return names;
    }

    bool fetch(variant_ostream& os) override
    {
        if (cols_.empty())
            columns();
        if (cols_.empty())
            return false;
        auto r = SQLFetch(stmt_.get());
        if (SQL_NO_DATA == r)
            return false;
        check(stmt_, r);

        for (size_t i = 0; i < cols_.size(); ++i)
            check(stmt_, cols_[i]->write(stmt_.get(), SQLUSMALLINT(i + 1), os));
        return true;
    }

    void set_autocommit(bool autocommit) override
    {
        reset_stmt(nullptr);
        if (get_autocommit() == autocommit)
            return;
        if (autocommit)
            check(dbc_, SQLEndTran(SQL_HANDLE_DBC, dbc_.get(), SQL_ROLLBACK));
        set_attr(dbc_,
                 SQL_ATTR_AUTOCOMMIT,
                 autocommit ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF);
    }

    void commit() override
    {
        reset_stmt(nullptr);
        if (!get_autocommit())
            check(dbc_, SQLEndTran(SQL_HANDLE_DBC, dbc_.get(), SQL_COMMIT));
    }

    std::string dbms_name() const { return get_info(dbc_, SQL_DBMS_NAME); }

private:
    detail::env_holder env_;
    detail::dbc_holder dbc_;
    detail::stmt_holder stmt_;
    std::string sql_;
    std::vector<detail::column_holder> cols_;

    void reset_stmt(SQLHANDLE stmt)
    {
        using namespace std::chrono;

        cols_.clear();
        sql_.clear();
        stmt_.reset(stmt);
        if (stmt_)
            set_attr(stmt_,
                     SQL_ATTR_QUERY_TIMEOUT,
                     duration_cast<seconds>(DbTimeout).count());
    }

    void prepare(std::string_view sql)
    {
        if (sql_ == sql)
            return;
        reset_stmt(alloc_handle(dbc_, SQL_HANDLE_STMT));
        auto ws = unicode::to_string<SQLWCHAR>(sql);
        check(stmt_, SQLPrepareW(stmt_.get(), (SQLWCHAR*)ws.c_str(), SQL_NTS));
        sql_ = sql;
    }

    template <class VariantViews>
    [[nodiscard]] std::vector<detail::binding_holder> bind_params(
        const VariantViews& params) {
        std::vector<detail::binding_holder> res;
        SQLSMALLINT num_params = 0;
        SQLNumParams(stmt_.get(), &num_params);
        auto count = std::max<size_t>(num_params, params.size());
        for (size_t i = 0; i < count; ++i) {
            auto bnd =
                detail::bind_param(i < params.size() ? &params[i] : nullptr);
            check(stmt_,
                  SQLBindParameter(stmt_.get(),
                                   SQLUSMALLINT(i + 1),
                                   bnd->input_output_type(),
                                   bnd->c_type(),
                                   bnd->sql_type(),
                                   column_size(*bnd),
                                   0,
                                   bnd->val_ptr(),
                                   0,
                                   bnd->ind()));
            res.push_back(std::move(bnd));
        }
        return res;
    }

    bool get_autocommit()
    {
        SQLUINTEGER attr = 0;
        SQLINTEGER len = sizeof(attr);
        check(dbc_,
              SQLGetConnectAttr(
                  dbc_.get(), SQL_ATTR_AUTOCOMMIT, &attr, len, &len));
        return SQL_AUTOCOMMIT_ON == attr;
    }

    SQLSMALLINT num_result_cols()
    {
        SQLSMALLINT res = 0;
        if (stmt_)
            check(stmt_, SQLNumResultCols(stmt_.get(), &res));
        return res;
    }

    void more_results()
    {
        auto r = num_result_cols() ? SQL_NO_DATA : SQL_SUCCESS;
        while (SQL_NO_DATA != r) {
            check(stmt_, r);
            r = SQLMoreResults(stmt_.get());
        }
    }
};

}  // namespace bark::db::odbc

#endif  // BARK_DB_ODBC_COMMAND_HPP
