TARGET =rmoexception

TEMPLATE = lib

DEFINES = RMOEXCEPTION_LIBRARY

# CONFIG += debug

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../lib

QT = core

INCLUDEPATH += /usr/include

SOURCES = \
    rmoexception.cpp 

HEADERS = \
    rmoexception.h 

# install
target.path = /opt/Qt/qtcreator-2.8.1/bin/plugins/designer
INSTALLS += target


