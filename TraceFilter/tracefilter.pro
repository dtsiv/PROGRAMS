TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = QExceptionDialog \
          QIniSettings \
          QPropPages \
          QSqlModel \
          QFilterWindow \
          FilterMain

win32 {
    !system(del /F FilterMain.exe)
}
