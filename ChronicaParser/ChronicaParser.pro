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

win32 {
    QMAKE_CXXFLAGS += /wd4244 /wd4018 /wd4200 /wd4189
    !system(del /F ChronicaParser.exe)
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
    main.cpp \
    parser.cpp \
    poi.cpp \
    ran1.cpp \
    sort.cpp \
    sqlmodel.cpp \
    intfspec.cpp \
    poi20191231.cpp

HEADERS += \
    parser.h \
    poi.h \
    qchrprotoacm.h \
    sqlmodel.h
