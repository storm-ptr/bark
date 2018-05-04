[![Build Status](https://travis-ci.org/storm-ptr/bark.svg?branch=master)](https://travis-ci.org/storm-ptr/bark)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/storm-ptr/bark?svg=true&branch=master)](https://ci.appveyor.com/project/storm-ptr/bark/branch/master)
[![Latest GitHub Release](https://img.shields.io/github/release/storm-ptr/bark.svg)](https://github.com/storm-ptr/bark/releases/latest)

## Introduction

Bark is a geospatial, cross-platform, C++ 14, header only library.

Submodules:
- db: reading and writing of raster and vector geospatial data
- geometry: convertion between Boost.Geometry, WKB and WKT
- proj: WKB reprojection from one coordinate reference system to another
- qt: visualization of raster and vector geospatial data

## example/nanogis

![](https://user-images.githubusercontent.com/3381451/38042411-f93918b8-32bc-11e8-8be0-433668c62d42.png)

## Getting Started

windows
- [how to set up the environment](https://github.com/storm-ptr/bark/blob/master/readme.windows.txt)
- [how to test](https://github.com/storm-ptr/bark/blob/master/test/readme.windows.txt)
- how to run example/nanogis: download [nanogis.windows.zip](https://github.com/storm-ptr/bark/releases/latest) for windows 10, extract and run
  ```
  ./nanogis/vc_redist.x64.exe
  ./nanogis/nanogis.exe
  ```

ubuntu
- [how to set up the environment](https://github.com/storm-ptr/bark/blob/master/readme.ubuntu.txt)
- [how to test](https://github.com/storm-ptr/bark/blob/master/test/readme.ubuntu.txt)
- how to run example/nanogis: download debian package for [ubuntu 14.04](https://github.com/storm-ptr/bark/releases/latest) or [ubuntu 18.04](https://yadi.sk/d/V1coAS6C3VKnGd) and run
  ```
  sudo dpkg -i ./nanogis.ubuntu.1804.deb
  sudo apt-get -f install
  nanogis
  ```


vmware
- [virtual machine with databases for testing](https://yadi.sk/d/sdEDsIjC3TkeM6)

andrew.naplavkov@gmail.com
