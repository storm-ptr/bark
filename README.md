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

![](https://user-images.githubusercontent.com/3381451/38042411-f93918b8-32bc-11e8-8be0-433668c62d42.png)

[download](https://github.com/storm-ptr/bark/releases/latest)

#### windows
  extract and run

  ```
    ./nanogis/vc_redist.x64.exe
    ./nanogis/nanogis.exe
  ```

#### ubuntu
<table><tr><th>16.04</th><th>18.04</th></tr><tr><td>

  ```
    sudo add-apt-repository ppa:beineri/opt-qt-5.12.0-xenial
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo add-apt-repository ppa:ubuntugis/ppa
    sudo apt-get update
    sudo dpkg -i ./nanogis.ubuntu.1604.deb
  ```
</td><td>

  ```
    sudo add-apt-repository ppa:beineri/opt-qt-5.12.0-bionic
    sudo add-apt-repository ppa:ubuntugis/ubuntugis-unstable
    sudo apt-get update
    sudo dpkg -i ./nanogis.ubuntu.1804.deb
  ```
</td></tr><tr><td colspan="2">

  ```
    sudo apt-get install -f
    nanogis
  ```
</td></tr></table>

## How to set up the development environment

#### windows

  * OSGeo4W<br>
  PowerShell
  ```
    mkdir C:\OSGeo4W64
    Invoke-WebRequest -Uri http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe -OutFile C:\OSGeo4W64\osgeo4w-setup-x86_64.exe
    C:\OSGeo4W64\osgeo4w-setup-x86_64.exe -q -k -r -A -s http://download.osgeo.org/osgeo4w/ -a x86_64 -P curl,gdal,libmysql,libmysql-devel,libpq,proj,spatialite,sqlite3 -R C:\OSGeo4W64
  ```
  environment variables<br>
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
  * copy [boost](https://www.boost.org/users/download/) library to ```C:\OSGeo4W64\include\boost```
  * Catch2 library<br>
  PowerShell
  ```
    Invoke-WebRequest -Uri https://raw.githubusercontent.com/catchorg/Catch2/v2.0.1/single_include/catch.hpp -OutFile C:\OSGeo4W64\include\catch.hpp
  ```
  * install [Microsoft Visual C++](https://www.visualstudio.com/vs/cplusplus/) (Community)
  * install [Qt](https://www.qt.io/download) libraries and creator (Open Source)
  * install [git](https://git-scm.com/downloads)
  * Bark library
  ```
    git clone --depth=1 https://github.com/storm-ptr/bark.git C:\OSGeo4W64\include\bark
  ```

#### ubuntu

<table><tr><th>16.04</th><th>18.04</th></tr><tr><td>

  ```
	sudo add-apt-repository ppa:beineri/opt-qt-5.12.0-xenial
	sudo add-apt-repository ppa:ubuntu-toolchain-r/test
	sudo add-apt-repository ppa:ubuntugis/ppa
	sudo apt-get update
	sudo apt-get -y install g++-8
  ```
</td><td>

  ```
	sudo add-apt-repository ppa:beineri/opt-qt-5.12.0-bionic
	sudo add-apt-repository ppa:ubuntugis/ubuntugis-unstable
	sudo apt-get update
  ```
</td></tr><tr><td colspan="2">

  ```
	sudo apt-get -y install git
	sudo apt-get -y install libboost-dev
	sudo apt-get -y install libgdal-dev
	sudo apt-get -y install libgl1-mesa-dev
	sudo apt-get -y install qt512-meta-minimal
	sudo apt-get -y install qt512imageformats
    wget https://raw.githubusercontent.com/catchorg/Catch2/v2.0.1/single_include/catch.hpp
    git clone --depth=1 https://github.com/storm-ptr/bark.git
  ```
</td></tr></table>

andrew.naplavkov@gmail.com
