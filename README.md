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
- how to run example/nanogis: download [debian package](https://github.com/storm-ptr/bark/releases/latest) for ubuntu 16.04 and run
  ```
  sudo add-apt-repository ppa:beineri/opt-qt-5.12.0-xenial
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo add-apt-repository ppa:ubuntugis/ppa
  sudo apt-get update
  sudo dpkg -i ./nanogis.ubuntu.1604.deb
  sudo apt-get -f install
  nanogis
  ```


vmware
- [virtual machine with databases for testing](https://yadi.sk/d/sdEDsIjC3TkeM6)

andrew.naplavkov@gmail.com
