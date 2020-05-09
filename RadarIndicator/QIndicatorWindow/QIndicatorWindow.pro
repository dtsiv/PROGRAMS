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

INCLUDEPATH +=  \
    ../include \
    ../QIniSettings \
    ../QPropPages \
    ../QExceptionDialog \
    ../QTargetsMap \
    ../QSqlModel \
    ../QRegFileParser \
    ../QPoi \
    ../include/nr2

FORMS +=

HEADERS += \
    qindicatorwindow.h \
    qparsemgr.h \
    usercontrolinputfilter.h \
    qsimumgr.h \
    qstopper.h \
    qnoisemapmgr.h
	
SOURCES += \
    qindicatorwindow.cpp \
    qparsemgr.cpp \
    usercontrolinputfilter.cpp \
    qsimumgr.cpp \
    qstopper.cpp \
    qnoisemapmgr.cpp

INCLUDEPATH +=  \
    ../include/nr2

