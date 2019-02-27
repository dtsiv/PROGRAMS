TEMPLATE = lib

 CONFIG += debug

TARGET   = qrmowidgetplugin

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../lib

QT += gui

CONFIG += designer plugin

INCLUDEPATH += ../Codograms
INCLUDEPATH += ../include
INCLUDEPATH += ../QLocalControlQMLWidgetLib
INCLUDEPATH += ../QFIndicatorWidgetLib
INCLUDEPATH += ../QConsoleWidgetLib
INCLUDEPATH += /usr/include

QMAKE_CXXFLAGS += -Wno-unused-local-typedefs -Wno-unused-parameter \
                  -Wformat=0 -Wno-unused-variable -Wno-switch \
                  -Wno-unused-but-set-variable

SOURCES= qrmowidgetplugin.cpp

HEADERS= \
    qrmowidgetplugin.h

LIBS += -L../lib -lqlocalcontrolqmlwidget
LIBS += -L../lib -lqfindicatorwidget
LIBS += -L../lib -lqconsolewidget
LIBS += -L/usr/lib/x86_64-linux-gnu -lfftw3

# install
target.path = /opt/Qt/qtcreator-2.8.1/bin/plugins/designer
INSTALLS += target

