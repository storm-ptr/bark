put file https://github.com/philsquared/Catch/blob/master/single_include/catch.hpp
  to <include> path (C:\Programs\OSGeo4W64\include)

- run

SET CXXFLAGS=/DBARK_TEST_DATABASE&& nmake -f ./makefile.windows test

nmake -f ./makefile.windows clean
