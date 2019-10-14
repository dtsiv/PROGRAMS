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
win32 {
    DESTDIR = C:/PROGRAMS/ChronicaParser
}
CONFIG += c++11

!win32 {
    QMAKE_CXXFLAGS += -std=gnu++11 -Wno-literal-suffix \
                      -Wno-unused-local-typedefs \
                      -Wno-unused-variale \
                      -Wno-unused-value \
                      -Wno-sign-compare
}

TEMPLATE = app

INCLUDEPATH += ../include/nr2

SOURCES += \
    avevar.cpp \
    betacf.cpp \
    betai.cpp \
    four1.cpp \
    ftest.cpp \
    gammln.cpp \
    gasdev.cpp \
    justdoppler.cpp \
    main.cpp \
    parser.cpp \
    poi.cpp \
    poi20190409.cpp \
    poiraw.cpp \
    ran1.cpp \
    sort.cpp \
    sqlmodel.cpp

HEADERS += \
    parser.h \
    poi.h \
    qchrprotoacm.h \
    sqlmodel.h
