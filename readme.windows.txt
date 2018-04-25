- install Microsoft Visual C++

  https://www.visualstudio.com/vs/cplusplus/



- download and run OSGeo4W network installer

  http://trac.osgeo.org/osgeo4w/



- select libs

  curl
  expat
  freexl
  gdal
  geos
  hdf4
  hdf5
  iconv
  iconv-vc14
  libgeotiff
  libjpeg
  libjpeg12
  libkml
  liblwgeom
  libmysql
  libmysql-devel
  libpng
  libpq
  libtiff
  libxml2
  msvcrt
  msvcrt2008
  msvcrt2010
  msvcrt2012
  msvcrt2013
  msvcrt2015
  netcdf
  ogdi
  openjpeg
  openssl
  proj
  proj-datumgrid
  proj-hpgn
  spatialite
  sqlite3
  szip
  xerces-c
  xerces-c-vc10
  xz
  zlib



- add to environment variables

  [PATH]
  C:\OSGeo4W64\bin;

  [INCLUDE]
  C:\OSGeo4W64\include;
  C:\OSGeo4W64\include\libpq;
  C:\OSGeo4W64\include\mysql;

  [GDAL_DATA]
  C:\OSGeo4W64\share\gdal



- put boost headers to C:\OSGeo4W64\include\boost

  http://www.boost.org/



- install Qt libraries and Qt Creator

  https://www.qt.io/download



- put bark headers to C:\OSGeo4W64\include\bark

  git clone https://github.com/storm-ptr/bark.git
