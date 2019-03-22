#-------------------------------------------------
#
# Project created by QtCreator 2016-08-25T11:18:19
#
#-------------------------------------------------

TARGET = qexceptiondialog

TEMPLATE = lib

DEFINES = QEXCEPTIONDIALOG_LIBRARY

# CONFIG += debug
CONFIG += staticlib

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../lib

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
        qexceptiondialog.cpp

HEADERS  += \
        qexceptiondialog.h

FORMS    += qexceptiondialog.ui

RESOURCES = qexceptiondialog.qrc

# install
target.path = /opt/Qt/qtcreator-2.8.1/bin/plugins/designer
INSTALLS += target

LIBS += -L../lib -lrmoexception
