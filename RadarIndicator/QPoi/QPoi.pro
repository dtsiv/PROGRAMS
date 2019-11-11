DESTDIR = $$PWD/../lib

CONFIG -= debug
CONFIG += release
CONFIG += static

OBJECTS_DIR= $$PWD/../build/.obj
MOC_DIR = $$PWD/../build/.moc
RCC_DIR = $$PWD/../build/.rcc
UI_DIR = $$PWD/../build/.ui

QT += core gui sql opengl widgets

TEMPLATE = lib

QMAKE_CXXFLAGS += /std:c++17

QMAKE_CXXFLAGS += /wd4018 /wd4244

INCLUDEPATH += \
    ../QPropPages \
    ../QIniSettings \
    ../QExceptionDialog \
    ../include/nr2

HEADERS += \
    qchrprotoacm.h \
    qpoi.h
	
SOURCES += \
    qpoi.cpp \
    avevar.cpp \
    betacf.cpp \
    betai.cpp \
    four1.cpp \
    ftest.cpp \
    gammln.cpp \
    gasdev.cpp \
    ran1.cpp \
    sort.cpp
