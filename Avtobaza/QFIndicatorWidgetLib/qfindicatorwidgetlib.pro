#-------------------------------------------------
#
# Project created by QtCreator 2016-08-18T11:38:36
#
#-------------------------------------------------

QT       += core gui opengl xml

DEFINES = QFINDICATORWIDGET_LIBRARY

 CONFIG += debug

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib

TARGET = qfindicatorwidget

# CONFIG += staticlib

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../lib

QMAKE_CXXFLAGS += -Wno-unused-local-typedefs -Wno-unused-parameter \
                  -Wformat=0 -Wno-unused-variable -Wno-switch \
                  -Wno-parentheses 

INCLUDEPATH += ../Codograms
INCLUDEPATH += ../include
INCLUDEPATH += /usr/include
INCLUDEPATH += ../RmoExceptionLib

SOURCES += \
        qfindicatorwidget.cpp \
    findicatorline.cpp \
    ftrace.cpp \
    proppage.cpp \
    stdafx.cpp

HEADERS  += \
    qfindicatorwidget.h \
    findicatorline.h \
    ftrace.h \
    proppage.h \
    stdafx.h

FORMS += \
        proppage.ui

# install
target.path = /opt/Qt/qtcreator-2.8.1/bin/plugins/designer
INSTALLS += target
