DESTDIR = $$PWD/../lib

CONFIG -= debug
CONFIG += release
CONFIG += static

OBJECTS_DIR= $$PWD/../build/.obj
MOC_DIR = $$PWD/../build/.moc
RCC_DIR = $$PWD/../build/.rcc
UI_DIR = $$PWD/../build/.ui

QT += core gui sql opengl widgets

TEMPLATE = lib

QMAKE_CXXFLAGS += /std:c++17

INCLUDEPATH +=  \
    ../QIniSettings \
    ../QPropPages \
    ../QExceptionDialog \
    ../QSqlModel

FORMS +=

HEADERS += \
    qindicatorwindow.h	
	
SOURCES += \
    qindicatorwindow.cpp

INCLUDEPATH +=  \
    $$PWD/../../PostgreSQL_9.4_32bit \
    $$PWD/../../include \
    $$PWD/../../include/nr2 \
    .

LIBS += -L$$PWD/../../PostgreSQL_9.4_32bit \
    -lopengl32 \
    -lglu32
