// Andrew Naplavkov

#ifndef BARK_DB_SLIPPY_GOOGLE_HPP
#define BARK_DB_SLIPPY_GOOGLE_HPP

#include <bark/db/slippy/detail/layer.hpp>

namespace bark::db::slippy {

inline std::string google_url(char lyr, const tile& tl)
{
    static random_index subdomain_{4};
    return concat("http://mt",
                  char('0' + subdomain_()),
                  ".google.com/vt/lyrs=",
                  lyr,
                  "&x=",
                  tl.x,
                  "&y=",
                  tl.y,
                  "&z=",
                  tl.z);
}

struct google_hybrid : layer {
    qualified_name name() override { return id("google", "hybrid"); }
    std::string url(const tile& tl) override { return google_url('y', tl); }
};

struct google_map : layer {
    qualified_name name() override { return id("google", "map"); }
    std::string url(const tile& tl) override { return google_url('m', tl); }
};

struct google_satellite : layer {
    qualified_name name() override { return id("google", "satellite"); }
    std::string url(const tile& tl) override { return google_url('s', tl); }
};

}  // namespace bark::db::slippy

#endif  // BARK_DB_SLIPPY_GOOGLE_HPP
