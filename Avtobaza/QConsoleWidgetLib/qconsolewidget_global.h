#ifndef QCONSOLEWIDGETLIB_GLOBAL_H
#define QCONSOLEWIDGETLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef QCONSOLEWIDGET_LIBRARY
# define QCONSOLEWIDGET_EXPORT Q_DECL_EXPORT
#else
# define QCONSOLEWIDGET_EXPORT Q_DECL_IMPORT
#endif

#endif // QCONSOLEWIDGETLIB_GLOBAL_H