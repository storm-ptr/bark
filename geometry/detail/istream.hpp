// Andrew Naplavkov

#ifndef BARK_GEOMETRY_DETAIL_ISTREAM_HPP
#define BARK_GEOMETRY_DETAIL_ISTREAM_HPP

#include <bark/detail/wkb.hpp>
#include <bark/geometry/detail/utility.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/mpl/map.hpp>
#include <functional>

namespace bark::geometry::detail {

class istream {
    wkb::istream data_;
    linestring cached_path_;

public:
    using path_t = std::reference_wrapper<decltype(cached_path_)>;

    explicit istream(blob_view data) : data_{data} {}

    template <class T>
    auto read()
    {
        return T::accept(data_, *this);
    }

    point operator()(double x, double y) { return {x, y}; }

    path_t operator()(uint32_t count, wkb::path)
    {
        cached_path_.clear();
        cached_path_.reserve(count);
        return cached_path_;
    }

    void operator()(path_t& sum, const point& item, wkb::path)
    {
        sum.get().push_back(item);
    }

    template <class T>
    typename boost::mpl::at<
        boost::mpl::map<boost::mpl::pair<wkb::path, polygon>,
                        boost::mpl::pair<wkb::point, multi_point>,
                        boost::mpl::pair<wkb::linestring, multi_linestring>,
                        boost::mpl::pair<wkb::polygon, multi_polygon>,
                        boost::mpl::pair<wkb::geometry, geometry_collection>>,
        T>::type
    operator()(uint32_t, wkb::chain<T>)
    {
        return {};
    }

    template <class Sum, class Item, class T>
    void operator()(Sum& sum, const Item& item, T)
    {
        sum.push_back(item);
    }

    void operator()(polygon& sum, const path_t& item, wkb::chain<wkb::path>)
    {
        push_ring(sum, item.get());
    }

    template <uint32_t Code, class T>
    void operator()(wkb::tagged<Code, T>)
    {
    }

    template <class Result, class T>
    Result operator()(Result&& res, T)
    {
        return std::forward<Result>(res);
    }

    linestring operator()(const path_t& res, wkb::linestring) { return res; }

    template <class Result>
    geometry operator()(Result&& res, wkb::geometry)
    {
        return std::forward<Result>(res);
    }
};

}  // namespace bark::geometry::detail

#endif  // BARK_GEOMETRY_DETAIL_ISTREAM_HPP
