QT += core
QT -= gui

DESTDIR = $$PWD
OBJECTS_DIR= $$PWD/build/.obj
MOC_DIR = $$PWD/build/.moc
RCC_DIR = $$PWD/build/.rcc
UI_DIR = $$PWD/build/.ui

CONFIG += c++11

TARGET = PassiveRejection
CONFIG += console
CONFIG -= app_bundle

DEFINES += HAVE_LAPACK_CONFIG_H
DEFINES += LAPACK_COMPLEX_STRUCTURE

TEMPLATE = app

SOURCES += main.cpp \
    xcgeev.cpp \
    auxroutines.cpp \
    xdsbev.cpp \
    xdpbsv.cpp \
    hamming.cpp

LIBS += -L../lib \
    -llibblas \
    -lliblapack \
    -lliblapacke

HEADERS += \
    routines.h

INCLUDEPATH += \
    "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/include" \
    $$PWD/../include \
    $$PWD/../include/nr2
