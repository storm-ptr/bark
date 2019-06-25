// Andrew Naplavkov

#ifndef BARK_DB_PROVIDER_OPS_HPP
#define BARK_DB_PROVIDER_OPS_HPP

#include <bark/db/detail/table_def_ops.hpp>
#include <bark/db/provider.hpp>
#include <boost/lexical_cast.hpp>

namespace bark::db {

inline column_def column(provider& pvd, const qualified_name& col_nm)
{
    return *find(pvd.table(qualifier(col_nm)).columns, col_nm.back());
}

template <class Result, class Sql>
auto fetch_or_default(provider& pvd, const Sql& sql)
{
    auto cmd = pvd.make_command();
    exec(*cmd, sql);
    auto rows = fetch_all(*cmd);
    auto is = variant_istream{rows.data};
    return is.data.empty() ? Result{} : boost::lexical_cast<Result>(read(is));
}

}  // namespace bark::db

#endif  // BARK_DB_PROVIDER_OPS_HPP
