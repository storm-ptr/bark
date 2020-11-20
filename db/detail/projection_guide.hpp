// Andrew Naplavkov

#ifndef BARK_DB_PROJECTION_GUIDE_HPP
#define BARK_DB_PROJECTION_GUIDE_HPP

#include <bark/db/detail/utility.hpp>
#include <bark/proj/bimap.hpp>
#include <bark/proj/epsg.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include <string>

namespace bark::db {

template <class T>
class projection_guide {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    proj::bimap load_projection_bimap()
    {
        proj::bimap res;
        auto bld = builder(as_mixin());
        as_mixin().as_dialect().projections_sql(bld);
        auto rows = fetch_all(as_mixin(), bld);
        for (auto& row : select(rows))
            copy(row, res);
        return res.empty() ? proj::epsg() : res;
    }

    int load_projection(const qualified_name& col_nm, std::string_view type)
    {
        auto bld = builder(as_mixin());
        as_mixin().as_dialect().projection_sql(bld, col_nm, type);
        auto cmd = as_mixin().make_command();
        exec(*cmd, bld);
        return fetch_or(*cmd, 0);
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
    void copy(const std::vector<variant_t>& from, proj::bimap& to)
    {
        enum columns { Srid, Epsg, Proj4 };
        auto srid = boost::lexical_cast<int>(from[Srid]);
        std::string pj;
        if (!is_null(from[Epsg]))
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

}  // namespace bark::db

#endif  // BARK_DB_PROJECTION_GUIDE_HPP
