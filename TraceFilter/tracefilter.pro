TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = QExceptionDialog \
          QIniSettings \
          QGeoUtils \
          QPropPages \
          QSqlModel \
          QFilterWindow \
          FilterMain

win32 {
    !system(del /F TraceFilter.exe)
}
