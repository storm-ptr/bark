// Andrew Naplavkov

#ifndef BARK_DB_POSTGRES_COMMAND_HPP
#define BARK_DB_POSTGRES_COMMAND_HPP

#include <bark/db/command.hpp>
#include <bark/db/detail/transaction.hpp>
#include <bark/db/postgres/detail/bind_column.hpp>
#include <bark/db/postgres/detail/bind_param.hpp>
#include <bark/db/postgres/detail/common.hpp>
#include <sstream>
#include <stdexcept>

namespace bark {
namespace db {
namespace postgres {

class command : public db::command,
                private db::detail::transaction<db::postgres::command> {
    friend db::detail::transaction<db::postgres::command>;

public:
    command(string_view host,
            int port,
            string_view db,
            string_view usr,
            string_view pwd)
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
        res.marker = [](std::ostream& os, size_t param_order) {
            os << '$' << (param_order + 1);
        };
        return res;
    }

    command& exec(const sql_builder& bld) override
    {
        reset_res(nullptr);
        auto sql = bld.sql();
        auto params = bld.params();

        std::vector<detail::param_holder> binds;
        std::vector<Oid> types;
        std::vector<const char*> values;
        std::vector<int> lengths;
        std::vector<int> formats;
        for (size_t i = 0; i < params.size(); ++i) {
            binds.push_back(detail::bind_param(params[i]));
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
                               detail::PGRES_FORMAT_BINARY));

        auto r = PQresultStatus(res_.get());
        if (r != PGRES_COMMAND_OK && r != PGRES_TUPLES_OK && params.empty()) {
            reset_res(PQexec(con_.get(), sql.c_str()));
            r = PQresultStatus(res_.get());
        }

        check(con_, r == PGRES_COMMAND_OK || r == PGRES_TUPLES_OK);
        return *this;
    }

    std::vector<std::string> columns() override
    {
        cols_.clear();
        int cols = res_ ? PQnfields(res_.get()) : 0;

        std::vector<std::string> names;
        for (decltype(cols) i = 0; i < cols; ++i) {
            names.push_back(PQfname(res_.get(), i));
            cols_.push_back(detail::bind_column(PQftype(res_.get(), i)));
        }
        return names;
    }

    bool fetch(dataset::ostream& os) override
    {
        if (cols_.empty())
            columns();
        if (cols_.empty() || row_ >= PQntuples(res_.get()))
            return false;

        auto row = row_++;
        for (size_t i = 0; i < cols_.size(); ++i) {
            if (PQgetisnull(res_.get(), row, (int)i))
                os << boost::blank{};
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
    detail::connection_holder con_;
    detail::result_holder res_;
    std::vector<detail::column_holder> cols_;
    int row_ = 0;

    void reset_res(PGresult* res)
    {
        row_ = 0;
        cols_.clear();
        res_.reset(res);
    }
};

}  // namespace postgres
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_POSTGRES_COMMAND_HPP
