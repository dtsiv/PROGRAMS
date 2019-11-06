TARGET = RadarIndicator

DESTDIR = $$PWD/../

TEMPLATE = app

QMAKE_CXXFLAGS += /std:c++17

RadarIndicator.depends = $$PWD/../QIndicatorWindow

OBJECTS_DIR= $$PWD/../build/.obj
MOC_DIR = $$PWD/../build/.moc
RCC_DIR = $$PWD/../build/.rcc
UI_DIR = $$PWD/../build/.ui

QT += core gui widgets opengl sql

CONFIG -= debug
CONFIG += release
CONFIG += static

INCLUDEPATH +=  $PWD/include

INCLUDEPATH += \
    ../QIndicatorWindow \
    ../QIniSettings \
    ../QPropPages \
    ../QSqlModel \
    ../QTargetsMap \
    ../QPoi \
    ../QExceptionDialog

SOURCES += main.cpp

RESOURCES += \
    indicatormain.qrc

LIBS += -L..\lib \
    -lqindicatorwindow \
    -lqinisettings \
    -lqproppages \
    -lqsqlmodel \
    -lqtargetsmap \
    -lqexceptiondialog \
    -lqpoi

LIBS += \
    -lopengl32 \
    -lglu32

win32 {
   RC_FILE = indicatormain.rc
}

