TEMPLATE = lib

TARGET = qlocalcontrolqmlwidget

DEFINES = QLOCALCONTROLQMLWIDGET_LIBRARY

# CONFIG += staticlib
 CONFIG += debug

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../lib

QT += gui xml network declarative sql

QMAKE_CXXFLAGS += -Wno-unused-local-typedefs -Wno-unused-parameter \
                  -Wformat=0 -Wno-unused-variable -Wno-switch \
                  -Wno-unused-but-set-variable

INCLUDEPATH += ../Codograms

INCLUDEPATH += /usr/include

SOURCES = programmodel.cpp  \
          proppage.cpp  \
          qlocalcontrolqmlwidget.cpp  \
          stdafx.cpp \
          targetselection.cpp

HEADERS = \
          programmodel.h \
          stdafx.h \
          proppage.h \
          qlocalcontrolqmlwidget.h \
          targetselection.h

FORMS += \
    targetselection.ui \
    proppageqml.ui

RESOURCES = \
    qlocalcontrolqmlwidget.qrc

# install
target.path = /opt/Qt/qtcreator-2.8.1/bin/plugins/designer
INSTALLS += target

