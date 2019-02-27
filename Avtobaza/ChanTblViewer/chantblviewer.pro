TARGET = chantblviewer

 CONFIG += debug

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../Release

QT += xml

QMAKE_CXXFLAGS += -Wno-unused-local-typedefs -Wno-unused-parameter \
                  -Wformat=0 -Wno-unused-variable -Wno-switch \
                  -Wno-unused-but-set-variable 


SOURCES = \
	  chantblviewer.cpp \
      main.cpp \
      stringListmodel.cpp

HEADERS = \
	  chantblviewer.h \
      stringlistmodel.h

FORMS = viewerdialog.ui

INCLUDEPATH += ../Codograms 

RESOURCES += \
    chantblviewer.qrc


