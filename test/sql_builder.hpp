// Andrew Naplavkov

#ifndef BARK_TEST_SQL_BUILDER_HPP
#define BARK_TEST_SQL_BUILDER_HPP

#include <bark/db/sql_builder.hpp>
#include <bark/geometry/as_binary.hpp>
#include <boost/lexical_cast.hpp>

TEST_CASE("sql_builder_params")
{
    using namespace bark;
    using namespace bark::db;
    using namespace std::string_literals;

    sql_builder bld{nullptr, [](auto) { return "?"; }};
    bld << param{"Hello!"s} << param{0ll} << param{"Bark"} << param{1.}
        << param{geometry::as_binary({{-118, 26}, {-111, 33}})};
    CHECK(boost::lexical_cast<std::string>(list{bld.params(), ", "}) ==
          "Hello!, 0, Bark, 1, 93 bytes");
}

#endif  // BARK_TEST_SQL_BUILDER_HPP
