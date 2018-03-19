// Andrew Naplavkov

#ifndef BARK_TEST_WKT_HPP
#define BARK_TEST_WKT_HPP

/// @see https://en.wikipedia.org/wiki/Well-known_text

// clang-format off
const char* Wkt[] = {"POINT(10 10)",
                     // "POINT EMPTY",
                     "LINESTRING(10 10,20 20,30 40)",
                     "POLYGON((10 10,10 20,20 20,20 15,10 10))",
                     "MULTIPOINT((10 10),(20 20))",
                     // "MULTIPOINT(10 40,40 30,20 20,30 10)",
                     "MULTILINESTRING((10 10,20 20),(15 15,30 15))",
                     "MULTIPOLYGON(((10 10,10 20,20 20,20 15,10 10)),((60 60,70 70,80 60,60 60)))",
                     // "MULTIPOLYGON EMPTY"
                     "GEOMETRYCOLLECTION(POINT(10 10),POINT(30 30),"
                     "LINESTRING(15 15,20 20))"};
// clang-format on

#endif  // BARK_TEST_WKT_HPP
