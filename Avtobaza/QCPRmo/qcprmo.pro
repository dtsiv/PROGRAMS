TARGET = qcprmo

 CONFIG += debug

GeneratedFiles = ../Build/$$TARGET
!system([ -d "$$GeneratedFiles" ]):system(mkdir $$GeneratedFiles)
MOC_DIR = $$GeneratedFiles
UI_DIR  = $$GeneratedFiles
RCC_DIR = $$GeneratedFiles
OBJECTS_DIR = $$GeneratedFiles

DESTDIR = ../Release

QT += core gui xml network

QMAKE_CXXFLAGS += -Wno-unused-local-typedefs -Wno-unused-parameter \
                  -Wformat=0 -Wno-unused-variable -Wno-switch \
                  -Wno-unused-but-set-variable 

SOURCES = \
          stdafx.cpp \
          main.cpp \
          about.cpp \
          proppage.cpp \
          initialize.cpp \
          proppagedlg.cpp \
          qcprmo.cpp 

HEADERS = \
          stdafx.h \
          proppage.h \
          proppagedlg.h \
          qcprmo.h \
          initialize.h 

FORMS = qcprmo.ui \
        about.ui \
        rmopropdlg.ui \
        initialize.ui \
        proppage.ui

LIBS += -L../lib/ -lqrmoconnection -lqrmowidgetplugin \
                  -lqlocalcontrolqmlwidget -lqfindicatorwidget \
                  -lqconsolewidget -lrmoexception \
                  -lqexceptiondialog

INCLUDEPATH += ../Codograms 
INCLUDEPATH += ../include
INCLUDEPATH += ../QLocalControlQMLWidgetLib
INCLUDEPATH += ../QFIndicatorWidgetLib
INCLUDEPATH += ../QRmoConnectionLib
INCLUDEPATH += ../QConsoleWidgetLib
INCLUDEPATH += ../QExceptionDialogLib
INCLUDEPATH += ../RmoExceptionLib

RESOURCES += \
    qcprmo.qrc


