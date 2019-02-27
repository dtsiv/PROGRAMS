TARGET = speeddialog

TEMPLATE = app

 CONFIG += debug

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../Release

QT += network xml

QMAKE_CXXFLAGS += -Wno-unused-local-typedefs -Wno-unused-parameter \
                  -Wformat=0 -Wno-unused-variable -Wno-switch \
                  -Wno-unused-but-set-variable 

INCLUDEPATH += ../Codograms
INCLUDEPATH += /usr/include
INCLUDEPATH += ../QRmoConnectionLib
INCLUDEPATH += ../QExceptionDialogLib

SOURCES = main.cpp \
          receiver.cpp \
          sender.cpp \
          speeddialog.cpp 

FORMS = speeddialog.ui

HEADERS = \
          receiver.h \
          sender.h \
          qexceptiondialog.h \
          speeddialog.h

RESOURCES = speeddialog.qrc

LIBS += -L../lib -lqrmoconnection -lrmoexception -lqexceptiondialog


