// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_PROJECTION_GUIDE_HPP
#define BARK_DB_DETAIL_PROJECTION_GUIDE_HPP

#include <bark/db/detail/common.hpp>
#include <bark/proj/bimap.hpp>
#include <bark/proj/epsg.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>
#include <exception>
#include <string>

namespace bark {
namespace db {
namespace detail {

template <typename T>
class projection_guide {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    proj::bimap load_projection_bimap()
    {
        auto bld = builder(as_mixin());
        as_mixin().as_dialect().projections_sql(bld);
        auto rows = fetch_all(as_mixin(), bld);
        if (rows.empty())
            return proj::epsg();
        proj::bimap res;
        for (auto& row : rows)
            copy(row, res);
        return res;
    }

    int load_projection(const qualified_name& col_nm,
                        boost::string_view type_lcase)
    {
        auto bld = builder(as_mixin());
        as_mixin().as_dialect().projection_sql(bld, col_nm, type_lcase);
        return fetch_or_default<int>(as_mixin(), bld);
    }

    int find_srid(const std::string& pj)
    {
        return as_mixin().cached_projection_bimap().find_srid(pj);
    }

    std::string find_proj(int srid)
    {
        return as_mixin().cached_projection_bimap().find_proj(srid);
    }

private:
    void copy(const dataset::rowset::tuple& from, proj::bimap& to)
    {
        enum columns { Srid, Epsg, Proj4 };
        auto srid = boost::lexical_cast<int>(from[Srid]);
        std::string pj;
        if (!dataset::is_null(from[Epsg]))
            try {
                pj = proj::epsg().find_proj(
                    boost::lexical_cast<int>(from[Epsg]));
            }
            catch (const std::exception&) {
            }
        if (pj.empty())
            pj = boost::lexical_cast<std::string>(from[Proj4]);
        try {
            to.insert(srid, pj);
        }
        catch (const std::exception&) {
        }
    }
};

}  // namespace detail
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_DETAIL_PROJECTION_GUIDE_HPP
