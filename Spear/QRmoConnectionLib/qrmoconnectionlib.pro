TARGET = qrmoconnection

TEMPLATE = lib

DEFINES = QRMOCONNECTION_LIBRARY

# CONFIG += debug

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../lib

QT += network xml

QMAKE_CXXFLAGS += -Wno-unused-local-typedefs -Wno-unused-parameter \
                  -Wformat=0 -Wno-unused-variable -Wno-switch \
                  -Wno-unused-but-set-variable -Wno-sign-compare

INCLUDEPATH += ../rmoexceptionlib

INCLUDEPATH += /usr/include

SOURCES = \
      qrmoconnection.cpp \
	  connectionthreadworker.cpp \
	  recvthreadworker.cpp \
   	  sendthreadworker.cpp

HEADERS = \
      qrmoconnection.h \
      qrmoconnection_p.h \
      connectionthreadworker.h \
	  recvthreadworker.h \
	  sendthreadworker.h

# install
target.path = /opt/Qt/qtcreator-2.8.1/bin/plugins/designer
INSTALLS += target

LIBS += -L../lib -lrmoexception
