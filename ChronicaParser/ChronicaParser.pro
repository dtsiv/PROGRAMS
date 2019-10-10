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

CONFIG += c++11

QMAKE_CXXFLAGS += -std=gnu++11 -Wno-literal-suffix \
                  -Wno-unused-local-typedefs \
                  -Wno-unused-variale \
                  -Wno-unused-value \
                  -Wno-sign-compare

TEMPLATE = app

INCLUDEPATH += ../include/nr2

SOURCES += main.cpp \
    parser.cpp \
    sqlmodel.cpp \
    poi.cpp \
    four1.cpp \
    poiraw.cpp \
    justdoppler.cpp \
    sort.cpp \
    ftest.cpp \
    avevar.cpp \
    gasdev.cpp \
    betacf.cpp \
    ran1.cpp \
    gammln.cpp \
    betai.cpp \
    poi20190409.cpp

HEADERS += \
    parser.h \
    qchrprotoacm.h \
    sqlmodel.h \
    poi.h
