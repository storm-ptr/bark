[![Build Status](https://travis-ci.org/storm-ptr/bark.svg?branch=master)](https://travis-ci.org/storm-ptr/bark)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/storm-ptr/bark?svg=true&branch=master)](https://ci.appveyor.com/project/storm-ptr/bark/branch/master)
[![Latest GitHub Release](https://img.shields.io/github/release/storm-ptr/bark.svg?)](https://github.com/storm-ptr/bark/releases/latest)

## Introduction

Bark is a geospatial, cross-platform, C++17, header only library.

Submodules:
- db: reading and writing of raster and vector geospatial data
- geometry: convertion between Boost.Geometry, WKB and WKT
- proj: WKB reprojection from one coordinate reference system to another
- qt: visualization of raster and vector geospatial data

## Documentation

* [doxygen](https://storm-ptr.github.io/bark/)

## How to run example/nanogis

![](https://user-images.githubusercontent.com/3381451/86533362-4bf8d200-bed9-11ea-85d5-fa3b00b674cc.png)

[download](https://github.com/storm-ptr/bark/releases/latest)

#### ubuntu 20.04

  ```
    sudo add-apt-repository -y ppa:beineri/opt-qt-5.15.2-focal
    sudo add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable
    sudo apt-get update
    sudo dpkg -i ./nanogis.ubuntu.2004.deb
    sudo apt-get install -f
    nanogis
  ```

#### windows 10
  extract

  ```
    ./nanogis/vc_redist.x64.exe
    ./nanogis/nanogis.exe
  ```

## How to set up the development environment

#### ubuntu 20.04

  ```
    sudo add-apt-repository -y ppa:beineri/opt-qt-5.15.2-focal
    sudo add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable
    sudo apt-get update
    sudo apt-get -y install g++
    sudo apt-get -y install git
    sudo apt-get -y install libboost-dev
    sudo apt-get -y install libgdal-dev
    sudo apt-get -y install libgl1-mesa-dev
    sudo apt-get -y install make
    sudo apt-get -y install qt515base
    sudo apt-get -y install qt515imageformats
    source /opt/qt515/bin/qt515-env.sh
    wget https://github.com/catchorg/Catch2/releases/download/v2.13.3/catch.hpp
    git clone --depth=1 https://github.com/storm-ptr/bark.git
  ```

#### windows 10
* OSGeo4W (PowerShell)
  ```
    mkdir C:\OSGeo4W64
    Invoke-WebRequest -Uri http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe -OutFile C:\OSGeo4W64\osgeo4w-setup-x86_64.exe
    C:\OSGeo4W64\osgeo4w-setup-x86_64.exe -q -k -r -A -s http://download.osgeo.org/osgeo4w/ -a x86_64 -P curl,gdal,libmysql,libmysql-devel,libpq,proj,spatialite,sqlite3 -R C:\OSGeo4W64
  ```
* set environment variable ```GDAL_DATA``` to ```C:\OSGeo4W64\share\gdal```
* set environment variable ```PROJ_LIB``` to ```C:\OSGeo4W64\share\proj```
* set environment variable ```INCLUDE``` to ```C:\OSGeo4W64\include;C:\OSGeo4W64\include\libpq;C:\OSGeo4W64\include\mysql```
* set environment variable ```LIB``` to ```C:\OSGeo4W64\lib```
* set environment variable ```PATH``` to ```C:\OSGeo4W64\bin```
* copy [boost](https://www.boost.org/users/download/) headers to ```C:\OSGeo4W64\include\boost```
* Catch2 library (PowerShell)
  ```
    Invoke-WebRequest -Uri https://github.com/catchorg/Catch2/releases/download/v2.13.3/catch.hpp -OutFile C:\OSGeo4W64\include\catch.hpp
  ```
* install [Microsoft Visual C++](https://www.visualstudio.com/vs/cplusplus/) (Community)
* install [Qt](https://www.qt.io/download) libraries and creator (Open Source)
* install [git](https://git-scm.com/downloads)
* install Bark library
  ```
    git clone --depth=1 https://github.com/storm-ptr/bark.git C:\OSGeo4W64\include\bark
  ```

andrew.naplavkov@gmail.com
