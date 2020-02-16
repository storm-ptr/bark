// Andrew Naplavkov

#ifndef BARK_DB_SQLITE_COMMAND_HPP
#define BARK_DB_SQLITE_COMMAND_HPP

#include <bark/db/command.hpp>
#include <bark/db/detail/transaction.hpp>
#include <bark/db/sqlite/detail/utility.hpp>
#include <boost/algorithm/string.hpp>
#include <stdexcept>

namespace bark::db::sqlite {

class command : public db::command, private transaction<sqlite::command> {
    friend transaction<sqlite::command>;

public:
    explicit command(const std::string& file)
    {
        constexpr int Flags =
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
        sqlite3* con = nullptr;
        check(con_, sqlite3_open_v2(file.c_str(), &con, Flags, nullptr));
        con_.reset(con);
        spatial_.reset(spatialite_alloc_connection());
        spatialite_init_ex(con_.get(), spatial_.get(), 0);
    }

    sql_syntax syntax() override { return {}; }

    void exec(const sql_builder& bld) override
    {
        auto sql = bld.sql();
        auto params = bld.params();
        boost::trim_right(sql);
        sqlite3_stmt* stmt = nullptr;
        const char* tail = nullptr;
        check(con_,
              sqlite3_prepare_v2(
                  con_.get(), sql.c_str(), (int)sql.size(), &stmt, &tail));
        stmt_.reset(stmt);
        if (strlen(tail)) {
            stmt_.reset(nullptr);
            if (!params.empty())
                throw std::runtime_error("SQLite params in multistatement");
            check(con_, sqlite3_exec(con_.get(), sql.c_str(), 0, 0, 0));
        }
        else {
            for (int i = 1; i <= (int)params.size(); ++i) {
                auto viz = overloaded{
                    [=](std::monostate) { return sqlite3_bind_null(stmt, i); },
                    [=](int64_t v) { return sqlite3_bind_int64(stmt, i, v); },
                    [=](double v) { return sqlite3_bind_double(stmt, i, v); },
                    [=](std::string_view v) {
                        return sqlite3_bind_text(
                            stmt, i, v.data(), int(v.size()), SQLITE_TRANSIENT);
                    },
                    [=](blob_view v) {
                        return sqlite3_bind_blob(
                            stmt, i, v.data(), int(v.size()), SQLITE_TRANSIENT);
                    }};
                check(con_, std::visit(std::move(viz), params[i - 1]));
            }
            step();
        }
    }

    std::vector<std::string> columns() override
    {
        std::vector<std::string> res;
        int cols = stmt_ ? sqlite3_column_count(stmt_.get()) : 0;
        for (int i = 0; i < cols; ++i)
            res.emplace_back(sqlite3_column_name(stmt_.get(), i));
        return res;
    }

    bool fetch(variant_ostream& os) override
    {
        if (!stmt_)
            return false;
        int cols = sqlite3_column_count(stmt_.get());
        for (int i = 0; i < cols; ++i)
            switch (sqlite3_column_type(stmt_.get(), i)) {
                case SQLITE_INTEGER:
                    os << (int64_t)sqlite3_column_int64(stmt_.get(), i);
                    break;
                case SQLITE_FLOAT:
                    os << sqlite3_column_double(stmt_.get(), i);
                    break;
                case SQLITE_TEXT:
                    os << std::string_view{
                        (char*)sqlite3_column_text(stmt_.get(), i),
                        (size_t)sqlite3_column_bytes(stmt_.get(), i)};
                    break;
                case SQLITE_BLOB:
                    os << blob_view{
                        (std::byte*)sqlite3_column_blob(stmt_.get(), i),
                        (size_t)sqlite3_column_bytes(stmt_.get(), i)};
                    break;
                default:
                    os << variant_t{};
            }
        step();
        return true;
    }

    void set_autocommit(bool autocommit) override
    {
        transaction::set_autocommit(autocommit);
    }

    void commit() override { transaction::commit(); }

private:
    connection_holder con_;
    spatialite_holder spatial_;
    statement_holder stmt_;

    void step()
    {
        auto r = sqlite3_step(stmt_.get());
        if (SQLITE_DONE == r)
            stmt_.reset(nullptr);
        check(con_, r);
    }
};

}  // namespace bark::db::sqlite

#endif  // BARK_DB_SQLITE_COMMAND_HPP
