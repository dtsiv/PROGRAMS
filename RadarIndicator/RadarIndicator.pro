# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Add-in.
# ------------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

CONFIG -= import_plugins

SUBDIRS = QExceptionDialog \
          QPoi \
          QSqlModel \
          QRegFileParser \
          QPropPages \
          QIniSettings \
          QTargetsMap \
          QIndicatorWindow \
          IndicatorMain

QMAKE_RC = rcc

win32 {
    !system(del /F RadarIndicator.exe)
}
