DESTDIR = $$PWD/../lib

CONFIG -= debug
CONFIG += release
CONFIG += static

OBJECTS_DIR= $$PWD/../build/.obj
MOC_DIR = $$PWD/../build/.moc
RCC_DIR = $$PWD/../build/.rcc
UI_DIR = $$PWD/../build/.ui

QT += core gui widgets

TEMPLATE = lib

QMAKE_CXXFLAGS += /std:c++17

INCLUDEPATH += \
    ../include \
    ../QExceptionDialog \
    ../QIniSettings \
    ../QPropPages

HEADERS += \
    tdcord.h \
    geocent.h \
    qpostsview.h \
    qgeoutils.h

SOURCES += \
    tdcord.cpp \
    geocent.cpp \
    qpostsview.cpp \
    qgeoutils.cpp
