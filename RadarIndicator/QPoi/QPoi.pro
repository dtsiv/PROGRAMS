DESTDIR = $$PWD/../lib

CONFIG -= debug
CONFIG += release
CONFIG += static

OBJECTS_DIR= $$PWD/../build/.obj
MOC_DIR = $$PWD/../build/.moc
RCC_DIR = $$PWD/../build/.rcc
UI_DIR = $$PWD/../build/.ui

QT += core gui sql widgets

TEMPLATE = lib

QMAKE_CXXFLAGS += /std:c++17

QMAKE_CXXFLAGS += /wd4018 /wd4244

INCLUDEPATH += \
    ../QPropPages \
    ../QIniSettings \
    ../QExceptionDialog \
    $$PWD/../../include/nr2

HEADERS += \
    qpoi.h \
    qnoisemap.h
	
SOURCES += \
    qpoi.cpp \
    four1.cpp \
    gasdev.cpp \
    ran1.cpp \
    sort.cpp \
    qnoisemap.cpp \
    dopplerrep.cpp \
    detecttargets.cpp \
    peleng.cpp
