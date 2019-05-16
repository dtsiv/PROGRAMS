# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Add-in.
# ------------------------------------------------------
DESTDIR = $$PWD
TEMPLATE = app
TARGET = QTraceFilter
QT += core gui sql opengl widgets
CONFIG += release
CONFIG += static

INCLUDEPATH +=  \
    ../PostgreSQL_9.4_32bit \
    ../include \
    ../include/nr2 \
    .
LIBS += -L$$PWD\..\PostgreSQL_9.4_32bit \
    -lopengl32 \
    -lglu32 \
    -llibpq

RESOURCES += \
    qtracefilter.qrc

FORMS += \
    qexceptiondialog.ui \
    qproppages.ui \
    qtracefilter.ui

HEADERS += \
    codograms.h \
    matrix.h \
    qavtctrl.h \
    qexceptiondialog.h \
    qexceptiondialog_global.h \
    qgenerator.h \
    qgeoutils.h \
    qindicator.h \
    qkalmanfilter.h \
    qpoimodel.h \
    qproppages.h \
    qserial.h \
    qtracefilter.h \
    qvoiprocessor.h \
    resource.h \
    rmoexception.h \
    rmoexception_global.h \
    tpoit.h

SOURCES += \
    gasdev.cpp \
    main.cpp \
    pythag.cpp \
    qexceptiondialog.cpp \
    qgenerator.cpp \
    qgeoutils.cpp \
    qindicator.cpp \
    qkalmanfilter.cpp \
    qpoimodel.cpp \
    qproppages.cpp \
    qserial.cpp \
    qtracefilter.cpp \
    qtracefilter01.cpp \
    qvoiprocessor.cpp \
    ran1.cpp \
    rmoexception.cpp \
    tpoit.cpp \
    tqli.cpp \
    tred2.cpp

win32 {
   RC_FILE = qtracefilter.rc
}
