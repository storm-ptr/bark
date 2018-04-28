- download OSGeo4W

  mkdir C:\OSGeo4W64

  (PowerShell) Invoke-WebRequest -Uri http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe -OutFile C:\OSGeo4W64\osgeo4w-setup-x86_64.exe

  C:\OSGeo4W64\osgeo4w-setup-x86_64.exe -q -k -r -A -s http://download.osgeo.org/osgeo4w/ -a x86_64 -P curl,gdal,libmysql,libmysql-devel,libpq,proj,spatialite,sqlite3 -R C:\OSGeo4W64

- add to environment variables

  [GDAL_DATA]
  C:\OSGeo4W64\share\gdal

  [INCLUDE]
  C:\OSGeo4W64\include
  C:\OSGeo4W64\include\libpq
  C:\OSGeo4W64\include\mysql

  [LIB]
  C:\OSGeo4W64\lib

  [PATH]
  C:\OSGeo4W64\bin

- put boost headers to C:\OSGeo4W64\include\boost

  https://www.boost.org/users/download/

- download Catch2 library

  (PowerShell) Invoke-WebRequest -Uri https://raw.githubusercontent.com/catchorg/Catch2/v2.0.1/single_include/catch.hpp -OutFile C:\OSGeo4W64\include\catch.hpp

- install git and download bark library

  https://git-scm.com/downloads

  git clone --depth=1 https://github.com/storm-ptr/bark.git C:\OSGeo4W64\include\bark

- install Microsoft Visual C++ (Community)

  https://www.visualstudio.com/vs/cplusplus/

- install Qt libraries and Qt Creator (Open Source)

  https://www.qt.io/download
