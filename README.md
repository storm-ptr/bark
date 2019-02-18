[![Build Status](https://travis-ci.org/storm-ptr/bark.svg?branch=master)](https://travis-ci.org/storm-ptr/bark)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/storm-ptr/bark?svg=true&branch=master)](https://ci.appveyor.com/project/storm-ptr/bark/branch/master)
[![Latest GitHub Release](https://img.shields.io/github/release/storm-ptr/bark.svg)](https://github.com/storm-ptr/bark/releases/latest)

## Introduction

Bark is a geospatial, cross-platform, C++17, header only library.

Submodules:
- db: reading and writing of raster and vector geospatial data
- geometry: convertion between Boost.Geometry, WKB and WKT
- proj: WKB reprojection from one coordinate reference system to another
- qt: visualization of raster and vector geospatial data

## example/nanogis

![](https://user-images.githubusercontent.com/3381451/38042411-f93918b8-32bc-11e8-8be0-433668c62d42.png)

## Getting Started

windows

* <details><summary>how to run example/nanogis</summary><p>

  download [nanogis.windows.zip](https://github.com/storm-ptr/bark/releases/latest) for windows 10, extract and run
  ```
  ./nanogis/vc_redist.x64.exe
  ./nanogis/nanogis.exe
  ```
  </p></details>
* <details><summary>how to set up the development environment</summary><p>

  - download OSGeo4W
  
    PowerShell
    ```
    mkdir C:\OSGeo4W64
    Invoke-WebRequest -Uri http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe -OutFile C:\OSGeo4W64\osgeo4w-setup-x86_64.exe
    C:\OSGeo4W64\osgeo4w-setup-x86_64.exe -q -k -r -A -s http://download.osgeo.org/osgeo4w/ -a x86_64 -P curl,gdal,libmysql,libmysql-devel,libpq,proj,spatialite,sqlite3 -R C:\OSGeo4W64
    ```

  - add environment variables
  
    GDAL_DATA
    ```
    C:\OSGeo4W64\share\gdal
    ```
    INCLUDE
    ```
    C:\OSGeo4W64\include
    C:\OSGeo4W64\include\libpq
    C:\OSGeo4W64\include\mysql
    ```
    LIB
    ```
    C:\OSGeo4W64\lib
    ```
    PATH
    ```
    C:\OSGeo4W64\bin
    ```

  - put [boost](https://www.boost.org/users/download/) headers to C:\OSGeo4W64\include\boost

  - download Catch2 library

    PowerShell
    ```
    Invoke-WebRequest -Uri https://raw.githubusercontent.com/catchorg/Catch2/v2.0.1/single_include/catch.hpp -OutFile C:\OSGeo4W64\include\catch.hpp
    ```

  - install [git](https://git-scm.com/downloads) and clone bark library

    ```
    git clone --depth=1 https://github.com/storm-ptr/bark.git C:\OSGeo4W64\include\bark
    ```

  - install [Microsoft Visual C++](https://www.visualstudio.com/vs/cplusplus/) (Community)

  - install [Qt](https://www.qt.io/download) libraries and creator (Open Source)

  </p></details>
* <details><summary>how to test</summary><p>

  - start the [virtual machine](https://yadi.sk/d/sdEDsIjC3TkeM6) with databases

  - run in C:\OSGeo4W64\include\bark\test

    ```
    SET CXXFLAGS=/DBARK_TEST_DATABASE&& nmake -f ./makefile.windows test
    nmake -f ./makefile.windows clean
    ```
  </p></details>

ubuntu 16.04
* <details><summary>how to run example/nanogis</summary><p>

  download [debian package](https://github.com/storm-ptr/bark/releases/latest) for ubuntu 16.04 and run
  ```
  sudo add-apt-repository ppa:beineri/opt-qt-5.12.0-xenial
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo add-apt-repository ppa:ubuntugis/ppa
  sudo apt-get update
  sudo dpkg -i ./nanogis.ubuntu.1604.deb
  sudo apt-get -f install
  nanogis
  ```
  </p></details>
* <details><summary>how to set up the development environment</summary><p>
  
  - install packages
      ```
      sudo add-apt-repository ppa:beineri/opt-qt-5.12.0-xenial
      sudo add-apt-repository ppa:ubuntu-toolchain-r/test
      sudo add-apt-repository ppa:ubuntugis/ppa
      sudo apt-get update
      sudo apt-get install g++-8
      sudo apt-get install git
      sudo apt-get install libboost-dev
      sudo apt-get install libgdal-dev
      sudo apt-get install libgl1-mesa-dev
      sudo apt-get install libproj-dev
      sudo apt-get install qt512-meta-minimal
      sudo apt-get install qt512imageformats
      ```
  
  - install odbc drivers

    ibm db2: download and extract

    [mssql](https://docs.microsoft.com/en-us/sql/connect/odbc/linux-mac/installing-the-microsoft-odbc-driver-for-sql-server)

    [mysql](https://dev.mysql.com/downloads/connector/odbc/)

    postgres: ```apt-get install odbc-postgresql```

  - check

    ```
    cat /etc/odbcinst.ini
  
    [ODBC Driver 17 for SQL Server]
    Description=Microsoft ODBC Driver 17 for SQL Server
    Driver=/opt/microsoft/msodbcsql17/lib64/libmsodbcsql-17.0.so.1.1
    UsageCount=1

    [IBM DATA SERVER DRIVER for ODBC]
    Description=IBM DATA SERVER DRIVER for ODBC - /home/dev/clidriver
    Driver=/home/dev/clidriver/lib/libdb2o.so.1
    UsageCount=1

    [MySQL ODBC 5.3 Unicode Driver]
    Description=MySQL ODBC 5.3 Unicode Driver - /home/dev/mysql-connector-odbc
    Driver=/home/dev/mysql-connector-odbc/lib/libmyodbc5w.so
    UsageCount=1

    [PostgreSQL ANSI]
    Description=PostgreSQL ODBC driver (ANSI version)
    Driver=psqlodbca.so
    Setup=libodbcpsqlS.so
    Debug=0
    CommLog=1
    UsageCount=1

    [PostgreSQL Unicode]
    Description=PostgreSQL ODBC driver (Unicode version)
    Driver=psqlodbcw.so
    Setup=libodbcpsqlS.so
    Debug=0
    CommLog=1
    UsageCount=1
    ```

  - download Catch2 library

    ```
    wget https://raw.githubusercontent.com/catchorg/Catch2/v2.0.1/single_include/catch.hpp
    ```

  - clone bark library

    ```
    git clone --depth=1 https://github.com/storm-ptr/bark.git
    ```
  </p></details>
* <details><summary>how to test</summary><p>

  - start the [virtual machine](https://yadi.sk/d/sdEDsIjC3TkeM6) with databases

  - run in bark/test

    ```
    make -f ./makefile.ubuntu test CXX=g++-8 CXXFLAGS+=-DBARK_TEST_DATABASE
    make -f ./makefile.ubuntu clean
    ```
  </p></details>

andrew.naplavkov@gmail.com
