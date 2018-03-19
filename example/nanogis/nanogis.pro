QT = concurrent core gui widgets
SOURCES = *.cpp
HEADERS = *.h
RESOURCES = resource.qrc
CONFIG += c++14

win32:INCLUDEPATH += C:\Programs\OSGeo4W64\include

win32:LIBS += -LC:\Programs\OSGeo4W64\lib\
  -lgdal_i\
  -llibcurl\
  -llibmysql\
  -llibpq\
  -lodbc32\
  -lproj_i\
  -lspatialite_i\
  -lsqlite3_i\
