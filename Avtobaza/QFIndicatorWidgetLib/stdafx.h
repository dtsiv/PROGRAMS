#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

//******************************************************************************
//
//******************************************************************************
#ifndef __linux
#include <windows.h>
#define snprintf _snprintf

#ifndef DBGPPRINT
#define DBGPPRINT
inline void DbgPrint(const wchar_t * fmt, ...)
{   wchar_t buf[512];
    va_list list;

    va_start(list, fmt);
    vswprintf_s(buf, 512, fmt, list);
    va_end(list);
    OutputDebugString(buf);
}
#endif
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
