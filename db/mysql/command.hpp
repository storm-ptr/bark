// Andrew Naplavkov

#ifndef BARK_DB_MYSQL_COMMAND_HPP
#define BARK_DB_MYSQL_COMMAND_HPP

#include <bark/db/command.hpp>
#include <bark/db/detail/transaction.hpp>
#include <bark/db/mysql/detail/bind_column.hpp>
#include <cstring>
#include <stdexcept>

namespace bark::db::mysql {

class command : public db::command, private transaction<mysql::command> {
    friend transaction<mysql::command>;

public:
    command(const std::string& host,
            int port,
            const std::string& db,
            const std::string& usr,
            const std::string& pwd)
    {
        using namespace std::chrono;
        constexpr auto Timeout =
            (unsigned int)duration_cast<seconds>(DbTimeout).count();
        con_.reset(mysql_init(0));
        if (!con_)
            throw std::runtime_error("MySQL error");
        for (auto opt : {MYSQL_OPT_CONNECT_TIMEOUT,
                         MYSQL_OPT_READ_TIMEOUT,
                         MYSQL_OPT_WRITE_TIMEOUT})
            check(con_, mysql_options(con_.get(), opt, &Timeout));
        check(con_,
              mysql_real_connect(con_.get(),
                                 host.c_str(),
                                 usr.c_str(),
                                 pwd.c_str(),
                                 db.c_str(),
                                 port,
                                 0,
                                 CLIENT_MULTI_STATEMENTS) == con_.get());
        check(con_, mysql_set_character_set(con_.get(), "utf8"));
    }

    sql_syntax syntax() override
    {
        sql_syntax res{};
        res.delimited_identifier = [](const auto& id) {
            return '`' + id + '`';
        };
        return res;
    }

    void exec(const sql_builder& bld) override
    {
        auto sql = bld.sql();
        auto params = bld.params();
        auto r = prepare(sql);
        if (r != 0 && params.empty()) {
            reset_stmt(nullptr);
            check(con_, mysql_query(con_.get(), sql.c_str()));
            for (int r = 0; r >= 0; r = mysql_next_result(con_.get())) {
                check(con_, r);
                result_holder{mysql_store_result(con_.get())};
            }
        }
        else {
            check(stmt_, r);
            std::vector<MYSQL_BIND> binds(params.size());
            for (size_t i = 0; i < params.size(); ++i) {
                auto viz = overloaded{
                    [&](std::monostate) {
                        binds[i].buffer_type = MYSQL_TYPE_NULL;
                    },
                    [&](const int64_t& v) {
                        binds[i].buffer_type = code_of<int64_t>();
                        binds[i].buffer = (char*)&v;
                    },
                    [&](const double& v) {
                        binds[i].buffer_type = code_of<double>();
                        binds[i].buffer = (char*)&v;
                    },
                    [&](auto v) {
                        binds[i].buffer_type = code_of<decltype(v)>();
                        binds[i].buffer = (char*)v.data();
                        binds[i].buffer_length = (unsigned long)v.size();
                    }};
                std::visit(std::move(viz), params[i]);
            }
            if (!binds.empty())
                check(stmt_, mysql_stmt_bind_param(stmt_.get(), binds.data()));
            check(stmt_, mysql_stmt_execute(stmt_.get()));
        }
    }

    std::vector<std::string> columns() override
    {
        reset_cols();
        result_holder res{stmt_ ? mysql_stmt_result_metadata(stmt_.get())
                                : nullptr};
        unsigned cols = res ? mysql_num_fields(res.get()) : 0;

        std::vector<std::string> names(cols);
        binds_.resize(cols);
        memset(binds_.data(), 0, binds_.size() * sizeof(MYSQL_BIND));
        cols_.resize(cols);
        for (unsigned i = 0; i < cols; ++i) {
            auto fld = mysql_fetch_field_direct(res.get(), i);
            names[i] = fld->name;
            cols_[i] = bind_column(fld->type, binds_[i]);
        }
        if (cols)
            check(stmt_, mysql_stmt_bind_result(stmt_.get(), binds_.data()));
        return names;
    }

    bool fetch(variant_ostream& os) override
    {
        if (cols_.empty())
            columns();
        if (cols_.empty())
            return false;
        auto r = mysql_stmt_fetch(stmt_.get());
        if (MYSQL_NO_DATA == r)
            return false;
        check(stmt_, 1 != r);

        for (size_t i = 0; i < cols_.size(); ++i) {
            if (cols_[i]->resize())
                check(stmt_,
                      mysql_stmt_fetch_column(
                          stmt_.get(), binds_.data() + i, (unsigned)i, 0));
            cols_[i]->write(os);
        }
        return true;
    }

    void set_autocommit(bool autocommit) override
    {
        transaction::set_autocommit(autocommit);
    }

    void commit() override { transaction::commit(); }

private:
    connection_holder con_;
    statement_holder stmt_;
    std::string sql_;
    std::vector<MYSQL_BIND> binds_;
    std::vector<column_holder> cols_;

    void reset_cols()
    {
        cols_.clear();
        binds_.clear();
    }

    void reset_stmt(MYSQL_STMT* stmt)
    {
        reset_cols();
        sql_.clear();
        stmt_.reset(stmt);
    }

    int prepare(std::string_view sql)
    {
        if (sql_ == sql)
            return 0;
        reset_stmt(mysql_stmt_init(con_.get()));
        check(con_, !!stmt_);
        sql_ = sql;
        return mysql_stmt_prepare(
            stmt_.get(), sql.data(), (unsigned long)sql.size());
    }
};

}  // namespace bark::db::mysql

#endif  // BARK_DB_MYSQL_COMMAND_HPP
