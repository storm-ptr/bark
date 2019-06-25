// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_COMMAND_HPP
#define BARK_DB_POSTGRES_COMMAND_HPP

#include <bark/db/command.hpp>
#include <bark/db/detail/transaction.hpp>
#include <bark/db/postgres/detail/bind_column.hpp>
#include <bark/db/postgres/detail/bind_param.hpp>
#include <bark/db/postgres/detail/utility.hpp>
#include <sstream>
#include <stdexcept>

namespace bark::db::postgres {

class command : public db::command, private transaction<postgres::command> {
    friend transaction<postgres::command>;

public:
    command(std::string_view host,
            int port,
            std::string_view db,
            std::string_view usr,
            std::string_view pwd)
    {
        using namespace std::chrono;
        std::ostringstream con;
        con << "host='" << host << "' port='" << port << "' dbname='" << db
            << "' user='" << usr << "' password='" << pwd
            << "' connect_timeout=" << duration_cast<seconds>(DbTimeout).count()
            << " options='-c statement_timeout="
            << duration_cast<milliseconds>(DbTimeout).count()
            << "' client_encoding='UTF8'";
        con_.reset(PQconnectdb((char*)con.str().c_str()));
        if (!con_ || PQstatus(con_.get()) != CONNECTION_OK)
            throw std::runtime_error("Postgres error");
    }

    sql_syntax syntax() override
    {
        sql_syntax res{};
        res.parameter_marker = [](auto order) {
            return '$' + std::to_string(order + 1);
        };
        return res;
    }

    void exec(const sql_builder& bld) override
    {
        reset_res(nullptr);
        auto sql = bld.sql();
        auto params = bld.params();

        std::vector<param_holder> binds;
        std::vector<Oid> types;
        std::vector<const char*> values;
        std::vector<int> lengths;
        std::vector<int> formats;
        for (size_t i = 0; i < params.size(); ++i) {
            binds.push_back(bind_param(params[i]));
            auto bnd = binds.back().get();
            types.push_back(bnd->type());
            values.push_back(bnd->value());
            lengths.push_back(bnd->length());
            formats.push_back(bnd->format());
        }
        reset_res(PQexecParams(con_.get(),
                               sql.c_str(),
                               (int)params.size(),
                               types.data(),
                               values.data(),
                               lengths.data(),
                               formats.data(),
                               PGRES_FORMAT_BINARY));

        auto r = PQresultStatus(res_.get());
        if (r != PGRES_COMMAND_OK && r != PGRES_TUPLES_OK && params.empty()) {
            reset_res(PQexec(con_.get(), sql.c_str()));
            r = PQresultStatus(res_.get());
        }

        check(con_, r == PGRES_COMMAND_OK || r == PGRES_TUPLES_OK);
    }

    std::vector<std::string> columns() override
    {
        cols_.clear();
        int cols = res_ ? PQnfields(res_.get()) : 0;

        std::vector<std::string> names;
        for (decltype(cols) i = 0; i < cols; ++i) {
            names.push_back(PQfname(res_.get(), i));
            cols_.push_back(bind_column(PQftype(res_.get(), i)));
        }
        return names;
    }

    bool fetch(variant_ostream& os) override
    {
        if (cols_.empty())
            columns();
        if (cols_.empty() || row_ >= PQntuples(res_.get()))
            return false;

        auto row = row_++;
        for (size_t i = 0; i < cols_.size(); ++i) {
            if (PQgetisnull(res_.get(), row, (int)i))
                os << variant_t{};
            else
                cols_[i]->write(res_.get(), row, (int)i, os);
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
    result_holder res_;
    std::vector<column_holder> cols_;
    int row_ = 0;

    void reset_res(PGresult* res)
    {
        row_ = 0;
        cols_.clear();
        res_.reset(res);
    }
};

}  // namespace bark::db::postgres

#endif  // BARK_DB_POSTGRES_COMMAND_HPP
