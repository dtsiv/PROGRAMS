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

INCLUDEPATH += \
    ../QPropPages \
    ../QIniSettings \
    ../QExceptionDialog
#    ../../CPP_211/other

HEADERS += \
    qtargetsmap.h \	
    qformular.h \
    qtargetmarker.h
	
SOURCES += \
    qtargetsmap.cpp \
    qformular.cpp \
    qtargetmarker.cpp
#    ../../CPP_211/recipes/sort.cpp
