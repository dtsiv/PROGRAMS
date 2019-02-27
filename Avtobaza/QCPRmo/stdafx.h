#include <QtCore>

#ifndef __linux
    #include <windows.h>
    #include "qavtctrl.h"
    #include "codograms.h"
#else
    #include "qavtctrl_linux.h"
    #include "codograms_linux.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
//#include <winsock2.h>
//#include <Ws2tcpip.h>
//******************************************************************************
//
//******************************************************************************
#ifndef DBGPPRINT
#define DBGPPRINT
#ifndef __linux
inline void DbgPrint(const wchar_t * fmt, ...)
{   wchar_t buf[512];
    va_list list;

    va_start(list, fmt);
    vswprintf_s(buf, 512, fmt, list);
    va_end(list);
    OutputDebugString(buf);
}
#else
#include <QtDebug>
inline void DbgPrint(const wchar_t * fmt, ...)
{   //wchar_t buf[512];
    //va_list list;
    //
    //va_start(list, fmt);
    //qDebug(QString::fromWCharArray(fmt).toLatin1(),list);
    //va_end(list);
    //!!! This function fails due to -fshort-wchar !!! 
}
#endif // __linux
#endif
