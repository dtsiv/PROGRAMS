#include <QtDebug>
#include <QtCore>
#include <QByteArray>
#include <QString>

#include "rmoexception.h"

RmoException::RmoException(QString str) _GLIBCXX_USE_NOEXCEPT { 
    m_qsMsg = QString("RmoException: ")+str;
}

// copy constructor
//RmoException::RmoException(const RmoException& _Right) _GLIBCXX_USE_NOEXCEPT
//		: m_qsMsg(_Right.m_qsMsg) { } // construct by copying _Right

// assignment operator
//RmoException& RmoException::operator=(const RmoException& _Right) _GLIBCXX_USE_NOEXCEPT {	
//    // assign _Right
//	m_qsMsg = _Right.m_qsMsg;
//	return (*this);
//}

// virtual keyword in base class destructor is important to invoke 
// child class destructor for pointer or reference variable
RmoException::~RmoException() _GLIBCXX_USE_NOEXCEPT {}

// it is important to specify const modifier to enable polymorphic what()
const char * RmoException::what() const _GLIBCXX_USE_NOEXCEPT { 
    m_baLocalEnc=m_qsMsg.toLocal8Bit().data(); 
    return m_baLocalEnc.data(); 
} 


