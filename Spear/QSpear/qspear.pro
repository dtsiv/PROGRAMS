TARGET = qspear

TEMPLATE = app

# CONFIG += debug

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../Release

QT += core network xml widgets

#QMAKE_CXXFLAGS += -Wno-unused-local-typedefs -Wno-unused-parameter \
#                  -Wformat=0 -Wno-unused-variable -Wno-switch \
#                  -Wno-unused-but-set-variable

INCLUDEPATH += /usr/include
INCLUDEPATH += ../rmoexceptionlib
INCLUDEPATH += ../qrmoconnectionlib
INCLUDEPATH += ../qexceptiondialoglib
INCLUDEPATH += ../../libvlc-qt_1.1.1_handmade_Dima/include
INCLUDEPATH += %ProgramFiles(x86)%/Microsoft SDKs/Windows/7.1A/Include

SOURCES = main.cpp \
          qledindicator.cpp \
          qserial.cpp \
          qspeararrangecontrols.cpp \
          qspearslots.cpp \
          qspearmodel.cpp \
          qspearauxclasses.cpp \
          qspear.cpp

FORMS = qspear.ui

HEADERS = \
          codograms.h \
          rmoexception.h \
          qledindicator.h \
          qserial.h \
          qexceptiondialog.h \
          qspearmodel.h \
          qspear.h

RESOURCES = qspear.qrc

TRANSLATIONS = qspear_ru.ts

LIBS += -L../lib -lqrmoconnection -lrmoexception -lqexceptiondialog
LIBS += -LC:/PROGRAMS/vlc-3.0.6/sdk/lib -llibvlc -llibvlccore
LIBS += -LC:/PROGRAMS/libvlc-qt_1.1.1_handmade_Dima/lib -lVLCQtWidgets -lVLCQtCore
