QT = concurrent core gui widgets
SOURCES = *.cpp
HEADERS = *.h
RESOURCES = resource.qrc
CONFIG += c++17
unix:QMAKE_CXXFLAGS += -std=c++17 -fPIC -Wno-missing-field-initializers
DEFINES += ACCEPT_USE_OF_DEPRECATED_PROJ_API_H

windows:INCLUDEPATH += ../../..

unix:INCLUDEPATH +=\
  ../../..\
  /usr/include/gdal\
  /usr/include/mysql\
  /usr/include/postgresql\

windows:LIBS += -L$$(LIB)\
  -lgdal_i\
  -llibcurl\
  -llibmysql\
  -llibpq\
  -lodbc32\
  -lproj_i\
  -lspatialite_i\
  -lsqlite3_i\

unix:LIBS +=\
  -lcurl\
  -lgdal\
  -lmysqlclient\
  -lodbc\
  -lproj\
  -lpq\
  -lspatialite\
  -lsqlite3\
