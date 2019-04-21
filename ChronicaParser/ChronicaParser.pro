#-------------------------------------------------
#
# Project created by QtCreator 2019-03-23T16:33:16
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TARGET = ChronicaParser
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = C:/PROGRAMS/ChronicaParser
CONFIG += c++11

QMAKE_CXXFLAGS +=

TEMPLATE = app

INCLUDEPATH += ../include/nr2

SOURCES += main.cpp \
    parser.cpp \
    sqlmodel.cpp \
    poi.cpp \
    four1.cpp \
    poiraw.cpp \
    justdoppler.cpp \
    poi20190409.cpp \
    sort.cpp

HEADERS += \
    parser.h \
    qchrprotoacm.h \
    sqlmodel.h \
    poi.h
