[![Build Status](https://travis-ci.org/storm-ptr/bark.svg?branch=master)](https://travis-ci.org/storm-ptr/bark)

## Introduction

Bark is a geospatial, cross-platform, C++ 14, header only library.

Submodules:
- db: reading and writing of raster and vector geospatial data
- geometry: convertion between Boost.Geometry, WKB and WKT
- proj: WKB reprojection from one coordinate reference system to another
- qt: visualization of raster and vector geospatial data

## Getting Started

windows
- [how to set up the environment](https://github.com/storm-ptr/bark/blob/master/readme.windows.txt)
- [how to test](https://github.com/storm-ptr/bark/blob/master/test/readme.windows.txt)
- [examlpe/nanogis executable binaries](https://yadi.sk/d/KFRNZBlp3TkeKh)  
  download, extract and run
  ```
  ./nanogis/nanogis.exe
  ```

ubuntu
- [how to set up the environment](https://github.com/storm-ptr/bark/blob/master/readme.ubuntu.txt)
- [how to test](https://github.com/storm-ptr/bark/blob/master/test/readme.ubuntu.txt)
- [examlpe/nanogis debian package](https://yadi.sk/d/-2z8pLqy3Tsvdd)  
  download and run
  ```
  sudo dpkg -i ./nanogis.1803.deb
  sudo apt-get -f install
  nanogis
  ```


vmware
- [virtual machine with databases for testing](https://yadi.sk/d/sdEDsIjC3TkeM6)

## example/nanogis

![](https://user-images.githubusercontent.com/3381451/38042411-f93918b8-32bc-11e8-8be0-433668c62d42.png)

andrew.naplavkov@gmail.com
