// Andrew Naplavkov

#ifndef BARK_GEOMETRY_GRAMMAR_HPP
#define BARK_GEOMETRY_GRAMMAR_HPP

#include <bark/geometry/detail/utility.hpp>
#include <bark/geometry/geometry.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <type_traits>

#define BARK_GEOMETRY_GRAMMAR_CTOR(T)                                     \
    template <class Dumb = result_type,                                   \
              std::enable_if_t<std::is_same_v<Dumb, T>, void*> = nullptr> \
    grammar() : grammar::base_type(T##_tagged_text)                       \
    {                                                                     \
        init();                                                           \
    }

#define BARK_GEOMETRY_GRAMMAR_CHAIN_DO(item, actor) \
    '(' >> item[actor] >> *(',' >> item[actor]) >> ')'

#define BARK_GEOMETRY_GRAMMAR_CHAIN(item) \
    BARK_GEOMETRY_GRAMMAR_CHAIN_DO(item, phoenix::push_back(qi::_val, qi::_1))

namespace bark::geometry {

namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;

template <class T>
using rule = qi::rule<const char*, T(), qi::blank_type>;

/// BNF Productions for Two-Dimension Geometry WKT
template <class T>
class grammar : public qi::grammar<const char*, T(), qi::blank_type> {
    using result_type = T;

    rule<double> x;
    rule<double> y;
    rule<point> point_;
    rule<point> point_text;
    rule<point> point_tagged_text;
    rule<linestring> linestring_text;
    rule<linestring> linestring_tagged_text;
    rule<polygon> polygon_text;
    rule<polygon> polygon_tagged_text;
    rule<multi_point> multi_point_text;
    rule<multi_point> multi_point_tagged_text;
    rule<multi_linestring> multi_linestring_text;
    rule<multi_linestring> multi_linestring_tagged_text;
    rule<multi_polygon> multi_polygon_text;
    rule<multi_polygon> multi_polygon_tagged_text;
    rule<geometry_collection> geometry_collection_text;
    rule<geometry_collection> geometry_collection_tagged_text;
    rule<geometry> geometry_tagged_text;

    void init()
    {
        geometry_tagged_text =
            point_tagged_text | linestring_tagged_text | polygon_tagged_text |
            multi_point_tagged_text | multi_linestring_tagged_text |
            multi_polygon_tagged_text | geometry_collection_tagged_text;
        point_tagged_text = qi::no_case["point"] >> point_text;
        linestring_tagged_text = qi::no_case["linestring"] >> linestring_text;
        polygon_tagged_text = qi::no_case["polygon"] >> polygon_text;
        multi_point_tagged_text = qi::no_case["multipoint"] >> multi_point_text;
        multi_linestring_tagged_text =
            qi::no_case["multilinestring"] >> multi_linestring_text;
        multi_polygon_tagged_text =
            qi::no_case["multipolygon"] >> multi_polygon_text;
        geometry_collection_tagged_text =
            qi::no_case["geometrycollection"] >> geometry_collection_text;
        point_text = '(' >> point_ >> ')';
        point_ = x[phoenix::bind(&point::set<0>, qi::_val, qi::_1)] >>
                 y[phoenix::bind(&point::set<1>, qi::_val, qi::_1)];
        x = qi::double_;
        y = qi::double_;
        linestring_text = BARK_GEOMETRY_GRAMMAR_CHAIN(point_);
        polygon_text = BARK_GEOMETRY_GRAMMAR_CHAIN_DO(
            linestring_text, phoenix::bind(&push_ring, qi::_val, qi::_1));
        multi_point_text = BARK_GEOMETRY_GRAMMAR_CHAIN(point_text);
        multi_linestring_text = BARK_GEOMETRY_GRAMMAR_CHAIN(linestring_text);
        multi_polygon_text = BARK_GEOMETRY_GRAMMAR_CHAIN(polygon_text);
        geometry_collection_text =
            BARK_GEOMETRY_GRAMMAR_CHAIN(geometry_tagged_text);
    }

public:
    BARK_GEOMETRY_GRAMMAR_CTOR(geometry);
    BARK_GEOMETRY_GRAMMAR_CTOR(point);
    BARK_GEOMETRY_GRAMMAR_CTOR(linestring);
    BARK_GEOMETRY_GRAMMAR_CTOR(polygon);
    BARK_GEOMETRY_GRAMMAR_CTOR(multi_point);
    BARK_GEOMETRY_GRAMMAR_CTOR(multi_linestring);
    BARK_GEOMETRY_GRAMMAR_CTOR(multi_polygon);
    BARK_GEOMETRY_GRAMMAR_CTOR(geometry_collection);
};

}  // namespace bark::geometry

#endif  // BARK_GEOMETRY_GRAMMAR_HPP
