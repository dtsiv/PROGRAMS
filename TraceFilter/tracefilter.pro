TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = QExceptionDialog \
          QIniSettings \
          QPropPages \
          QFilterWindow \
          FilterMain

win32 {
    !system(del /F FilterMain.exe)
}
