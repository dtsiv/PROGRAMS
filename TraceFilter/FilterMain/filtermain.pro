TARGET = TraceFilter

DESTDIR = $$PWD/../

TEMPLATE = app

QMAKE_CXXFLAGS += /std:c++17

TraceFilter.depends = $$PWD/../QFilterWindow

OBJECTS_DIR= $$PWD/../build/.obj
MOC_DIR = $$PWD/../build/.moc
RCC_DIR = $$PWD/../build/.rcc
UI_DIR = $$PWD/../build/.ui

QT += core gui widgets opengl sql

CONFIG -= debug
CONFIG += release
CONFIG += static

INCLUDEPATH += \
    ../include/nr2 \
    ../QExceptionDialog \
    ../QIniSettings \
    ../QPropPages \
    ../QSqlModel \
    ../QFilterWindow

SOURCES += main.cpp

RESOURCES += \
    filtermain.qrc

LIBS += -L../lib \
    -lqinisettings \
    -lqproppages \
    -lqsqlmodel \
    -lqfilterwindow \
    -lqexceptiondialog

LIBS += \
    -L../PostgreSQL_9.4_32bit \
    -llibpq \
    -lopengl32 \
    -lglu32

win32 {
   RC_FILE = filtermain.rc
}

