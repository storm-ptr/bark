// Andrew Naplavkov

#ifndef BARK_PROJ_BIMAP_HPP
#define BARK_PROJ_BIMAP_HPP

#include <bark/proj/normalize.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <stdexcept>
#include <string>

namespace bark::proj {

class bimap {
public:
    bool empty() const { return index_.empty(); }

    void insert(int srid, const std::string& pj)
    {
        index_.insert({srid, normalize(pj)});
    }

    int find_srid(const std::string& pj) const
    {
        auto it = index_.right.find(normalize(pj));
        if (it == index_.right.end())
            throw std::out_of_range("projection: " + pj);
        return it->second;
    }

    std::string find_proj(int srid) const
    {
        auto it = index_.left.find(srid);
        if (it == index_.left.end())
            throw std::out_of_range("srid: " + std::to_string(srid));
        return it->second;
    }

private:
    boost::bimap<boost::bimaps::multiset_of<int>,
                 boost::bimaps::multiset_of<std::string>>
        index_;
};

}  // namespace bark::proj

#endif  // BARK_PROJ_BIMAP_HPP
