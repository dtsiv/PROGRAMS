DESTDIR = $$PWD/../

TEMPLATE = app

TARGET = RadarIndicator

RadarIndicator.depends = $$PWD/../QIndicatorWindow

OBJECTS_DIR= $$PWD/../build/.obj
MOC_DIR = $$PWD/../build/.moc
RCC_DIR = $$PWD/../build/.rcc
UI_DIR = $$PWD/../build/.ui

QT += core gui widgets opengl

CONFIG -= debug
CONFIG += release
CONFIG += static

INCLUDEPATH +=  $PWD/include

INCLUDEPATH += \
    ../QIndicatorWindow \
    $$UI_DIR

SOURCES += main.cpp

RESOURCES += \
    indicatormain.qrc

LIBS += -L..\lib \
    -lqindicatorwindow

LIBS += -L$$PWD/../../PostgreSQL_9.4_32bit \
    -lopengl32 \
    -lglu32

win32 {
   RC_FILE = indicatormain.rc
}

