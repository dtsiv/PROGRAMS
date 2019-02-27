#ifndef QRMOCONNECTION_GLOBAL_H
#define QRMOCONNECTION_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef QRMOCONNECTION_LIBRARY
# define QRMOCONNECTION_EXPORT Q_DECL_EXPORT
#else
# define QRMOCONNECTION_EXPORT Q_DECL_IMPORT
#endif

#endif // QRMOCONNECTION_GLOBAL_H
