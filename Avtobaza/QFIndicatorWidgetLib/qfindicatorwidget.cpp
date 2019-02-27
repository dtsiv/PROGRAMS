#include "stdafx.h"
#include <QMutex>
#include <QGLWidget>
#include <QtDebug>
#include <QLayout>
#include <QBoxLayout>
#include <QMetaObject>
#include <QDomDocument>

#include <math.h>
#include <stdio.h>

#include "qfindicatorwidget.h"
#include "proppage.h"
#include "findicatorline.h"
#include "ftrace.h"

#include "rmoexception.h"
class QFIndicatorException : public RmoException {
public: 
    QFIndicatorException(QString str) :
    RmoException(QString("F-Indicator: ") + str) {}    
};

#define VER_MAJOR 1
#define VER_MINOR 10

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068)
#endif

//******************************************************************************
//
//******************************************************************************
QFIndicatorWidget::QFIndicatorWidget(QWidget * parent) : QWidget(parent)
{
    m_pLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_pLayout -> setContentsMargins(4, 4, 4, 4);
    m_pLayout -> setSpacing(3);
    m_bRequestsEnab = true;
    m_pRequestTread = new TRequestTread(this);
}
//******************************************************************************
//
//******************************************************************************
QFIndicatorWidget::~QFIndicatorWidget()
{
    m_pRequestTread -> ThreadStop();
    delete m_pRequestTread;
    // DbgPrint(L"QFIndicatorWidget::DESTRUCTOR");
    while(m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.first();  
        m_FLineList.removeFirst();
        m_pLayout -> removeWidget((QWidget *)p);
        delete p;
    }
    delete m_pLayout;
}
//*****************************************************************************
//
//*****************************************************************************
void QFIndicatorWidget::lock(void) { m_Lock.lock(); }
//*****************************************************************************
//
//*****************************************************************************
void QFIndicatorWidget::unlock(void) { m_Lock.unlock(); }
//*****************************************************************************
//
//*****************************************************************************
QStringList QFIndicatorWidget::about()
{
    QStringList qsVer;
    QString qs;
#pragma GCC diagnostic ignored "-Wformat"
    qs.sprintf("QFIndicatorWidgetLib v.%ld.%02ld beta by Tristan 2012-2014", 
                      (DWORD)VER_MAJOR, (DWORD)VER_MINOR);
#pragma GCC diagnostic pop
    qsVer << qs;
    return(qsVer);
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::getPropPages(QList<QWidget *> * pwl, QWidget * pParent)
{
    FIndicatorPropPage * pwg = new FIndicatorPropPage(pParent, this);
    pwl -> append((QWidget *)pwg);
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::ApplyChanges(QList<TFIndicatorLineStyle *> * plist)
{
    lock();
    QList<TFIndicatorLine *> FLineList = m_FLineList;
    int i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.at(i++);  
        m_pLayout -> removeWidget((QWidget *)p);
    }
    m_FLineList.clear();
    i = 0;
    while(i < plist -> count())
    {   TFIndicatorLineStyle * pl = plist -> at(i++);
        TFIndicatorLine * p = new TFIndicatorLine(m_FLineList.count(), pl, this);
        m_FLineList.append(p);
        m_pLayout -> addWidget((QWidget *)p, 3);
    }
    unlock();
    rebuild();
    i = 0;
    while(i < FLineList.count()) delete FLineList.at(i++);  
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::initialize(QDomDocument * pDomProp)
{
    QDomNode nd;
    QString qs;
    bool bOk;

#pragma GCC diagnostic ignored "-Wformat"
    QDomNodeList ndl = pDomProp -> elementsByTagName("FIndicator"); 
    if(ndl.count() < 1) return;
    QDomElement eFIndicator = ndl.at(0).toElement();
    QDomNodeList ndll = eFIndicator.elementsByTagName("RequestsEnable");    
    if(ndll.count() > 0)
    {   nd = ndll.at(0);    
        ndl = nd.childNodes();
        if(ndl.count() > 0) 
        {   nd = ndl.at(0);
            qs = nd.toText().data();
            m_bRequestsEnab = qs.toInt();
        }
    }
    ndll = eFIndicator.elementsByTagName("FLine");  
    lock();
    while(m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.first();  
        m_FLineList.removeFirst();
        m_pLayout -> removeWidget((QWidget *)p);
        delete p;
    }
    int i = 0;
    while(i < ndll.count())
    {   QDomElement eFLine = ndll.at(i++).toElement();
    
        TFIndicatorLine * p = new TFIndicatorLine(m_FLineList.count(), this);
        m_FLineList.append(p);
        m_pLayout -> addWidget((QWidget *)p, 3);
    
        ndl = eFLine.elementsByTagName("Extent");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                double x, y;
                // swscanf(qs.utf16(), L"%lf %lf", &x, &y);
                sscanf(qs.toLocal8Bit(), "%lf %lf", &x, &y);
                p -> m_s.m_Extent = QPointF(x, y);
            }
        }
        ndl = eFLine.elementsByTagName("Offset");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                double x, y;
                // swscanf(qs.utf16(), L"%lf %lf", &x, &y);
                sscanf(qs.toLocal8Bit(), "%lf %lf", &x, &y);
                p -> m_s.m_Ofset = QPointF(x, y); 
            }
        }
        ndl = eFLine.elementsByTagName("BkgColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_BkgColor.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("VerticLineColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_clrLineVertic.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("VerticTextColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_clrTextVertic.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("HorizLineColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_clrLineHoriz.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("HorizTextColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_clrTextHoriz.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("FillColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_clrFill.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("OutlineColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_clrOutline.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("TextColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_clrText.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("TraceColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p ->m_s. m_clrTrace.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("PoitColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_clrPoit.setRgba(qs.toUInt(&bOk, 16));
            }
        }
        ndl = eFLine.elementsByTagName("PelengColor");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                unsigned int u0, u1, u2, u3;
                // swscanf(qs.utf16(), L"%lx %lx %lx %lx", &u0, &u1, &u2, &u3);
                sscanf(qs.toLocal8Bit(), "%lx %lx %lx %lx", &u0, &u1, &u2, &u3);
                p -> m_s.m_clrP0.setRgba(u0);
                p -> m_s.m_clrP1.setRgba(u1);
                p -> m_s.m_clrP2.setRgba(u2);
                p -> m_s.m_clrP3.setRgba(u3);
            }
        }
        ndl = eFLine.elementsByTagName("PositionsId");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                // swscanf(qs.utf16(), L"%ld %ld %ld %ld",
                //    &p -> m_s.m_PId0, &p -> m_s.m_PId1, &p -> m_s.m_PId2, &p -> m_s.m_PId3);
                sscanf(qs.toLocal8Bit(), "%ld %ld %ld %ld",
                    &p -> m_s.m_PId0, &p -> m_s.m_PId1, &p -> m_s.m_PId2, &p -> m_s.m_PId3);
            }
        }
        ndl = eFLine.elementsByTagName("YMax");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_dYMax = qs.toDouble();
            }
        }
        ndl = eFLine.elementsByTagName("FFrom");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_dFFrom = qs.toDouble();
            }
        }
        ndl = eFLine.elementsByTagName("FTo");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_dFTo = qs.toDouble();
            }
        }
        ndl = eFLine.elementsByTagName("AzFrom");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_dAzFrom = qs.toDouble();
            }
        }
        ndl = eFLine.elementsByTagName("AzTo");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_dAzTo = qs.toDouble();
            }
        }
        ndl = eFLine.elementsByTagName("ComPoints");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_iComPoints = qs.toInt();
            }
        }
        ndl = eFLine.elementsByTagName("DencityX");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_WebDencityX = qs.toInt();
            }
        }
        ndl = eFLine.elementsByTagName("DencityY");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_WebDencityY = qs.toInt();
            }
        }
        ndl = eFLine.elementsByTagName("TTL");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_iTTL = qs.toInt();
            }
        }
        ndl = eFLine.elementsByTagName("FormType");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_iFormType = qs.toInt();
            }
        }
        ndl = eFLine.elementsByTagName("FontW");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_WFont.fromString(qs);
            }
        }
        ndl = eFLine.elementsByTagName("FontF");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_FFont.fromString(qs);
            }
        }
        ndl = eFLine.elementsByTagName("EnableMask");
        if(ndl.count() > 0)
        {   nd = ndl.at(0); 
            ndl = nd.childNodes();
            if(ndl.count() > 0) 
            {   nd = ndl.at(0);
                qs = nd.toText().data();
                p -> m_s.m_EnabMask = (int)qs.toUInt(&bOk, 16);
            }
        }
    }
    unlock();
#pragma GCC diagnostic pop
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::getConfiguration(QDomDocument * pDomProp)
{
    if(pDomProp == NULL) return;

    int i;
    bool bPiPresent = false;
    bool bComPresent = false;
    QDomElement eParam;
    QString qs;

    QDomNodeList ndl = pDomProp -> childNodes();
    i = 0;
    while(i < ndl.count())
    {   QDomNode nd = ndl.at(i);
        QDomNode::NodeType ndt = nd.nodeType();
#pragma GCC diagnostic ignored "-Wswitch"
        switch(ndt)
        {   case QDomNode::ProcessingInstructionNode:
                bPiPresent = true;
                break;

            case QDomNode::CommentNode:
                bComPresent = true;
                break;

            case QDomNode::ElementNode:
                if(nd.toElement().tagName() == "PARAMETERS")
                eParam = nd.toElement(); 
                break;
        }
#pragma GCC diagnostic pop
        i++;
    }
    if(!bPiPresent)     
    {   QDomProcessingInstruction pi;
        pi = pDomProp -> createProcessingInstruction("xml", "version='1.0' encoding='UTF-8");
        pDomProp -> appendChild(pi); 
    }
    if(!bComPresent)
    {   QDomComment com;
        com = pDomProp -> createComment("QRmo parameters store file");
        pDomProp -> appendChild(com); 
    }
    if(eParam.isNull())
    {   eParam = pDomProp -> createElement("PARAMETERS");
        pDomProp -> appendChild(eParam);
    }
    qs = "FIndicator";
    QDomNodeList impl = eParam.elementsByTagName(qs);
    i = 0;
    while(i < impl.count()) eParam.removeChild(impl.at(i++));
    QDomElement eFIndicator = pDomProp -> createElement(qs);
    QDomElement e = pDomProp -> createElement("RequestsEnable");
    QDomText t = pDomProp -> createTextNode(qs.setNum((int)m_bRequestsEnab));
    e.appendChild(t);
    eFIndicator.appendChild(e);
    
#pragma GCC diagnostic ignored "-Wformat"
    lock();
    i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * pFLine = m_FLineList.at(i++);
        QDomElement eFLine = pDomProp -> createElement("FLine");
    
        e = pDomProp -> createElement("Extent");
        t = pDomProp -> createTextNode(qs.sprintf("%.9lf %.9lf", pFLine -> m_s.m_Extent.x(), pFLine -> m_s.m_Extent.y()));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("Offset");
        t = pDomProp -> createTextNode(qs.sprintf("%.3lf %.3lf", pFLine -> m_s.m_Ofset.x(), pFLine -> m_s.m_Ofset.y()));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("BkgColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_BkgColor.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("VerticLineColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrLineVertic.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("VerticTextColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrTextVertic.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("HorizLineColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrLineHoriz.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("HorizTextColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrTextHoriz.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("HorizTextColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrTextHoriz.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("FillColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrFill.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("OutlineColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrOutline.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("TextColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrText.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("TraceColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrTrace.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("PoitColor");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_clrPoit.rgba(), 16));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("PelengColor");
        t = pDomProp -> createTextNode(qs.sprintf("%08lx %08lx %08lx %08lx",
            pFLine -> m_s.m_clrP0.rgba(), pFLine -> m_s.m_clrP1.rgba(),
            pFLine -> m_s.m_clrP2.rgba(), pFLine -> m_s.m_clrP3.rgba()));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("PositionsId");
        t = pDomProp -> createTextNode(qs.sprintf("%ld %ld %ld %ld",
            pFLine -> m_s.m_PId0, pFLine -> m_s.m_PId1,
            pFLine -> m_s.m_PId2, pFLine -> m_s.m_PId3));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("YMax");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_dYMax, 'f', 3));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("FFrom");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_dFFrom, 'f', 3));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("FTo");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_dFTo, 'f', 3));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("AzFrom");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_dAzFrom, 'f', 3));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("AzTo");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_dAzTo, 'f', 3));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("ComPoints");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_iComPoints));
        e.appendChild(t);
        eFLine.appendChild(e);
        
        e = pDomProp -> createElement("DencityX");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_WebDencityX));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("DencityY");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_WebDencityY));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("TTL");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_iTTL));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("FormType");
        t = pDomProp -> createTextNode(qs.setNum(pFLine -> m_s.m_iFormType));
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("FontW");
        t = pDomProp -> createTextNode(pFLine -> m_s.m_WFont.toString());
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("FontF");
        t = pDomProp -> createTextNode(pFLine -> m_s.m_FFont.toString());
        e.appendChild(t);
        eFLine.appendChild(e);

        e = pDomProp -> createElement("EnableMask");
        t = pDomProp -> createTextNode(qs.setNum((unsigned int)pFLine -> m_s.m_EnabMask, 16));
        e.appendChild(t);
        eFLine.appendChild(e);
        eFIndicator.appendChild(eFLine);
    }
    unlock();
#pragma GCC diagnostic pop
    eParam.appendChild(eFIndicator);
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::EmitFSelection(int id, double dFSel, double dBandSel, int iCPCount, double dAzFrom, double dAzTo, int iFlags)
{
    emit SendFSelection(id, dFSel, dBandSel, iCPCount, dAzFrom, dAzTo, iFlags);
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::updateFrSel(int iRmoId, PFRSELECTION pFrSel, int iCou)
{   return;
    int i = 0;
    if(iCou == 0)
    {   lock();
        while(i < m_FLineList.count())
        {   TFIndicatorLine * p = m_FLineList.at(i++);
            p -> updateFrSel(NULL);
        }
        unlock();   
        return;
    }
    PFRSELECTION ps = pFrSel;
#pragma GCC diagnostic ignored "-Wsign-compare"
    while(i++ < iCou)
    {   if(ps -> rm == iRmoId)
        {   int j = 0;
            lock();
            while(j < m_FLineList.count())
            {   TFIndicatorLine * p = m_FLineList.at(j++);
                if(ps -> id == p -> id())
                {   p -> updateFrSel(ps);
                    break;
                }
            }
            unlock();   
        }
        ps++;
    }
#pragma GCC diagnostic pop
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::traceTbl(PTRACELISTENTRY ptle, int iCou)
{   return;
    m_pRequestTread -> AddRequest(ptle, iCou);
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::sendInfoReuqest(int iId, double dFrom, double dTo)
{
    if(m_bRequestsEnab) emit SendCommand(TRACEINFOREQ, iId, dFrom, dTo);
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::traceInfo(PTRACEINFOHEADER ptih)
{   return;
    lock();
//  if(!m_Lock.tryLock()) return;
    int i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.at(i++);
        if(p -> m_s.m_EnabMask & ENAB_SHOW_T) p -> AddPoints(ptih);
    }
    unlock();
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::traceInfoE(PTRACEINFOHEADER ptih)
{   return;
    lock();
//  if(!m_Lock.tryLock()) return;
    int i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.at(i++);
        if(p -> m_s.m_EnabMask & ENAB_SHOW_T) p -> AddPointsE(ptih);
    }
    unlock();
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::clear(void)
{
    lock();
    int i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.at(i++);
        p -> clear();
    }
    unlock();
    emit SendFSelection(0, 0., 0., 0, 0., 0., FRSEL_FLAG_CLEARALL);
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::AddPoit(PPOIT ppoit)
{   return;
    lock();
//  if(!m_Lock.tryLock()) return;
    int i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.at(i++);
        p -> AddPoints(ppoit);
    }
    unlock();
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::AddPoit(PPOITE ppoit)
{   return;
    lock();
    int i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.at(i++);
        p -> AddPoints(ppoit);
    }
    unlock();
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::AddPostt(PPOSTT ppt)
{   return;
    lock();
    int i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.at(i++);
        p -> AddPoints(ppt);
    }
    unlock();
}
//******************************************************************************
//
//******************************************************************************
void QFIndicatorWidget::AddRaw16(PSCOPE16C pscope) 
{   return;
    lock();
    int i = 0;
    while(i < m_FLineList.count())
    {   TFIndicatorLine * p = m_FLineList.at(i++);
        p -> AddScope(pscope);
    }
    unlock();
}
//*****************************************************************************
//
//*****************************************************************************
int QFIndicatorWidget::sizeofImp(PIMPINFO pii)
{   int iSize = sizeof(IMPINFO);
    if(pii -> sComboPointsCount) iSize += (pii -> sComboPointsCount + 1) * sizeof(DWORD);
    return(iSize);
}
//*****************************************************************************
//
//*****************************************************************************
int QFIndicatorWidget::sizeofStrob(PPOSTT ppt)
{   int iSize = sizeof(POSTT);
    PIMPINFO pi = (PIMPINFO)(ppt + 1);
    int i = 0;
    while(i++ < ppt -> iImpCount)
    {   iSize += sizeofImp(pi);
        pi = nextImp(pi);
    }
    return(iSize);
}
//*****************************************************************************
//
//*****************************************************************************
PIMPINFO QFIndicatorWidget::nextImp(PIMPINFO pii)
{
    char * p = (char *)pii;
    p += sizeofImp(pii);
    return((PIMPINFO)p);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
