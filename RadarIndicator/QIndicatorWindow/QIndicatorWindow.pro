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
    $$PWD/../include \
    ../QIniSettings \
    ../QPropPages \
    ../QExceptionDialog \
    ../QTargetsMap \
    ../QSqlModel \
    ../QRegFileParser \
    ../QPoi

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
    $$PWD/../../include/nr2

INCLUDEPATH += \
    "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/include"
