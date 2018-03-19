- put file https://github.com/philsquared/Catch/blob/master/single_include/catch.hpp
  to <include> path (C:\Programs\OSGeo4W64\include)

- compile and run main.cpp

[msvc]
SET INCLUDE=C:\Programs\OSGeo4W64\include;%INCLUDE%&& SET LIB=C:\Programs\OSGeo4W64\lib;%LIB%&& cl /EHsc /bigobj /std:c++latest /D_HAS_AUTO_PTR_ETC=1^
 main.cpp^
 second_translation_unit.cpp^
 gdal_i.lib^
 libcurl.lib^
 libmysql.lib^
 libpq.lib^
 odbc32.lib^
 proj_i.lib^
 spatialite_i.lib^
 sqlite3_i.lib
