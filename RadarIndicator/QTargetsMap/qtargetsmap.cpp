#include "qtargetsmap.h"
#include "qinisettings.h"
#include "qexceptiondialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QRegExp>
#include <QOpenGLFunctions>

// time delay to show formular
const quint64 QTargetsMap::m_iFormularDelay = 1000;

QTargetsMap::QTargetsMap(QWidget * pOwner /* = 0 */)
         : QObject(0)
         , m_dScaleD(2.0e0)
         , m_dMaxScaleD(QTARGETSMAP_MAXSCALED)
         , m_dMinScaleD(QTARGETSMAP_MINSCALED)
         , m_dScaleV(2.0e0)
         , m_dMaxScaleV(QTARGETSMAP_MAXSCALEV)
         , m_dMinScaleV(QTARGETSMAP_MINSCALEV)
         , m_dViewD0(-20)
         , m_dViewV0(200)
         , m_dDGridStep(20.0e0)
         , m_dVGridStep(20.0e0)
         , m_pSafeParams(NULL)
         , m_pOwner(pOwner)
         , m_pMouseStill(NULL)
         , m_uTimerMSecs(0)
         , m_bAdaptiveGridStep(false)
         , m_pMovingFormular(NULL) {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(QTARGETSMAP_SCALE_D,2.0e0);
    m_dScaleD = iniSettings.value(QTARGETSMAP_SCALE_D,scRes).toDouble();
    iniSettings.setDefault(QTARGETSMAP_SCALE_V,2.0e0);
    m_dScaleV = iniSettings.value(QTARGETSMAP_SCALE_V,scRes).toDouble();
    iniSettings.setDefault(QTARGETSMAP_VIEW_D0,-20.0e0);
    m_dViewD0 = iniSettings.value(QTARGETSMAP_VIEW_D0,scRes).toDouble();
    iniSettings.setDefault(QTARGETSMAP_VIEW_V0,150.0e0);
    m_dViewV0 = iniSettings.value(QTARGETSMAP_VIEW_V0,scRes).toDouble();
    iniSettings.setDefault(QTARGETSMAP_ADAPT_GRD_STEP,false);
    m_bAdaptiveGridStep = iniSettings.value(QTARGETSMAP_ADAPT_GRD_STEP,scRes).toBool();
    iniSettings.setDefault(QTARGETSMAP_GRD_STEP_D,20.0e0);
    m_dDGridStep = iniSettings.value(QTARGETSMAP_GRD_STEP_D,scRes).toDouble();
    iniSettings.setDefault(QTARGETSMAP_GRD_STEP_V,20.0e0);
    m_dVGridStep = iniSettings.value(QTARGETSMAP_GRD_STEP_V,scRes).toDouble();
    iniSettings.setDefault(QTARGETSMAP_TIMER_MSEC,100);
    m_uTimerMSecs = iniSettings.value(QTARGETSMAP_TIMER_MSEC,scRes).toUInt();

    // periodic formulars update
    QObject::connect(&m_qtFormular,SIGNAL(timeout()),SLOT(onFormularTimeout()));
    m_qtFormular.start(m_iFormularDelay);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QTargetsMap::~QTargetsMap() {
    // delete previously allocated dynamic objects
    if (m_pSafeParams) {
        delete m_pSafeParams;
        m_pSafeParams = 0;
    }
    while (m_qlTargets.count()) delete m_qlTargets.takeLast();
    // save module settings
    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setValue(QTARGETSMAP_SCALE_D, m_dScaleD);
    iniSettings.setValue(QTARGETSMAP_SCALE_V, m_dScaleV);
    iniSettings.setValue(QTARGETSMAP_VIEW_D0, m_dViewD0);
    iniSettings.setValue(QTARGETSMAP_VIEW_V0, m_dViewV0);
    iniSettings.setValue(QTARGETSMAP_ADAPT_GRD_STEP, m_bAdaptiveGridStep);
    iniSettings.setValue(QTARGETSMAP_GRD_STEP_D, m_dDGridStep);
    iniSettings.setValue(QTARGETSMAP_GRD_STEP_V, m_dVGridStep);
    iniSettings.setValue(QTARGETSMAP_TIMER_MSEC,m_uTimerMSecs);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::onFormularTimeout() {
    // clear stale formulars
    for (int i=0; i<m_qlFormulars.size(); i++ ) {
        if (m_qlFormulars.at(i)->isStale()) {
            delete m_qlFormulars.takeAt(i);
            emit doUpdate();
        }
    }
    // process location of frozen mouse
    if (!m_pMouseStill) return;
    if (m_pMouseStill->last.msecsTo(QTime::currentTime()) < m_iFormularDelay) return;
    QFormular *pFormular = new QFormular(m_pMouseStill);
    // coordinate transformation
    m_transform.reset();
    m_transform.scale(m_dScaleD,-m_dScaleV);
    m_transform.translate(-m_dViewD0,-m_dViewV0);
    // select closest target to display its formular
    if (!pFormular->selectTarg(m_qlTargets, m_transform)) { delete pFormular; return; }
    m_qlFormulars.append(pFormular);
    delete m_pMouseStill;
    m_pMouseStill = 0;
    emit doUpdate();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QGridLayout *pGridLayout=new QGridLayout;
    // grid step
    pGridLayout->addWidget(new QLabel("Grid step"),0,0);
    pPropPages->m_pcbAdaptiveGrid = new QCheckBox("adaptive");
    pPropPages->m_pcbAdaptiveGrid->setChecked(m_bAdaptiveGridStep);
    pGridLayout->addWidget(pPropPages->m_pcbAdaptiveGrid,0,1);
    // strob delay
    pGridLayout->addWidget(new QLabel("Strob delay (ms)"),0,3);
    pPropPages->m_pleTimerMSecs = new QLineEdit(QString::number(m_uTimerMSecs));
    pPropPages->m_pleTimerMSecs->setValidator(new QIntValidator(0,QTARGETSMAP_MAXTIMERMSEC,pPropPages->m_pleTimerMSecs));
    pGridLayout->addWidget(pPropPages->m_pleTimerMSecs,0,4);

    pGridLayout->setColumnStretch(2,100);
    pGridLayout->setColumnStretch(5,100);
    QVBoxLayout *pVLayout=new QVBoxLayout;
    pVLayout->addLayout(pGridLayout);
    pVLayout->addStretch();
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QTARGETSMAP_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::propChanged(QObject *pPropDlg) {
    bool bOk=false;
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    // assign new prop values
    m_bAdaptiveGridStep = pPropPages->m_pcbAdaptiveGrid->isChecked();
    quint32 uTimerMSecs = pPropPages->m_pleTimerMSecs->text().toUInt(&bOk);
    if (bOk) {
        m_uTimerMSecs = qBound((quint32)0,uTimerMSecs,(quint32)QTARGETSMAP_MAXTIMERMSEC);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MapWidget *QTargetsMap::getMapInstance() {
    static bool bInit=false;
    if (bInit) {
        throw RmoException("MapWidget already initialized");
        return NULL;
    }
    MapWidget *pMapWidget = new MapWidget(this);

    QObject::connect(this,SIGNAL(doUpdate()),pMapWidget,SLOT(update()));
    bInit = true;
    return pMapWidget;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::onExceptionDialogClosed() {
    // some QWidget shifts/rescales destry last mouse location
    if (m_pMouseStill) {
        delete m_pMouseStill;
        m_pMouseStill = NULL;
    }
    if (!m_pSafeParams) {
        m_pSafeParams = new struct GridSafeParams;
        QIniSettings &iniSettings = QIniSettings::getInstance();
        m_pSafeParams->dViewD0    = iniSettings.getDefault(QTARGETSMAP_VIEW_D0).toDouble();
        m_pSafeParams->dViewV0    = iniSettings.getDefault(QTARGETSMAP_VIEW_V0).toDouble();
        m_pSafeParams->dScaleD    = iniSettings.getDefault(QTARGETSMAP_SCALE_D).toDouble();
        m_pSafeParams->dScaleV    = iniSettings.getDefault(QTARGETSMAP_SCALE_V).toDouble();
        m_pSafeParams->dDGridStep = iniSettings.getDefault(QTARGETSMAP_GRD_STEP_D).toDouble();
        m_pSafeParams->dVGridStep = iniSettings.getDefault(QTARGETSMAP_GRD_STEP_V).toDouble();
    }
    m_dViewD0    = m_pSafeParams->dViewD0;
    m_dViewV0    = m_pSafeParams->dViewV0;
    m_dScaleD    = m_pSafeParams->dScaleD;
    m_dScaleV    = m_pSafeParams->dScaleV;
    m_dDGridStep = m_pSafeParams->dDGridStep;
    m_dVGridStep = m_pSafeParams->dVGridStep;
    // clear last error message
    m_qsLastError.clear();
    emit doUpdate();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::shiftView(bool bVertical, bool bPositive) {
    double dVShiftStep=2.0e0; // m/s
    double dDShiftStep=2.0e0;  // m
    if (bVertical) {
        m_dViewV0 += dVShiftStep*(-1+2*bPositive);
    }
    else {
        m_dViewD0 += dDShiftStep*(-1+2*bPositive);
    }
    // some QWidget shifts/rescales destry last mouse location
    if (m_pMouseStill) {
        delete m_pMouseStill;
        m_pMouseStill = NULL;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::shiftView(QPoint qpShift) {
    if (m_dScaleD) {
        m_dViewD0 -= qpShift.x()/m_dScaleD;
    }
    if (m_dScaleV) {
        m_dViewV0 += qpShift.y()/m_dScaleV;
    }
    // some QWidget shifts/rescales destry last mouse location
    if (m_pMouseStill) {
        delete m_pMouseStill;
        m_pMouseStill = NULL;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::shiftFormular(QPoint qpShift) {
    // qDebug() << "shiftFormular";
    if (m_pMovingFormular) {
        m_pMovingFormular->m_qpOffset+=qpShift;
        // qDebug() << "adding shift";
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::resetScale() {
    if (m_pSafeParams) {
        m_dScaleD    = m_pSafeParams->dScaleD;
        m_dScaleV    = m_pSafeParams->dScaleV;
        m_dViewD0    = m_pSafeParams->dViewD0;
        m_dViewV0    = m_pSafeParams->dViewV0;
        m_dDGridStep = m_pSafeParams->dDGridStep;
        m_dVGridStep = m_pSafeParams->dVGridStep;
    }
    // some QWidget shifts/rescales destry last mouse location
    if (m_pMouseStill) {
        delete m_pMouseStill;
        m_pMouseStill = NULL;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::mapPaintEvent(MapWidget *pMapWidget, [[maybe_unused]]QPaintEvent *qpeEvent) {

    // start rendering
    QPainter painter;
    painter.begin(pMapWidget);
    painter.setRenderHint(QPainter::Antialiasing);

    // OpenGL requires: clear all first
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // coordinate transformation
    m_transform.reset();
    m_transform.scale(m_dScaleD,-m_dScaleV);
    m_transform.translate(-m_dViewD0,-m_dViewV0);

    // coordinate grid
    if (m_qsLastError.isEmpty()) {
        if (!drawGrid(pMapWidget, painter)) {
            if (!m_pSafeParams) {
                m_qsLastError=QString("Grid safe parameters missing");
            }
            QExceptionDialog *pDlg = new QExceptionDialog(m_qsLastError, pMapWidget);
            QObject::connect(pDlg,SIGNAL(accepted()),SLOT(onExceptionDialogClosed()));
            pDlg -> setAttribute(Qt::WA_DeleteOnClose);
            pDlg -> open();
        }
        for (int i=0; i<m_qlFormulars.size(); i++ ) {
            m_qlFormulars.at(i)->drawFormular(painter,m_transform);
        }
    }

    // target markers
    for (int i=0; i<m_qlTargets.size(); i++) {
        m_qlTargets.at(i)->drawMarker(painter,m_transform);
    }
    painter.end();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QTargetsMap::drawGrid(MapWidget *pMapWidget, QPainter &painter) {
    // push painter settings on stack
    painter.save();

    // some constants
    int iHeight = painter.window().height();
    int iWidth  = painter.window().width();
    int iMargin = 2;
    int iLeftMargin = 2;
    QColor qcGridColor(Qt::darkGray);
    // grid steps in pixels
    double dVGridStepPix=m_dVGridStep*m_dScaleV;
    double dDGridStepPix=m_dDGridStep*m_dScaleD;
    QString qsPrecV; qsPrecV = QString::number(m_dVGridStep-qFloor(m_dVGridStep),'f').replace(QRegExp("(0|\\.|\\-|\\+)")," ").trimmed();
    // qDebug() << "qsPrecV = " << qsPrecV;
    int iPrecV = qsPrecV.length();
    QString qsPrecD; qsPrecD = QString::number(m_dDGridStep-qFloor(m_dDGridStep),'f').replace(QRegExp("(0|\\.|\\-|\\+)")," ").trimmed();
    // qDebug() << "qsPrecD = " << qsPrecD;
    int iPrecD = qsPrecD.length();

    int iGridThickness = 1;
    int iAxisThickness = 2;
    int iTickLabelSize=8;
    int iTickLabelOffsetX=5;
    int iTickLabelOffsetY=5;

    // font metrics
    QFont font=QApplication::font();
    font.setWeight(QFont::Normal);
    font.setPointSize(iTickLabelSize);
    painter.setFont(font);
    QFontMetrics fmTickLabel(font,pMapWidget);

    // colors and fonts
    QBrush qbGridBrush(qcGridColor);
    QPen qpGridPen(qbGridBrush,iGridThickness,Qt::SolidLine,Qt::RoundCap);
    painter.setPen(qpGridPen);

    // Position of physical coordinate origin in pixels (view coordinates)
    double dViewD0Pix = - m_dViewD0 * m_dScaleD;
    double dViewV0Pix = + m_dViewV0 * m_dScaleV;

    //------------------------------------------------------------------------
    // all coordinates declared here are in widget coordinate system
    // i.e. (0,0) is left upper corner
    //------------------------------------------------------------------------
    // screen corners in widget coordinates
    double dX1,dY1, // left upper
           dX2,dY2, // right upper
           dX3,dY3, // right lower
           dX4,dY4; // left lower
    dX1=dX4=iLeftMargin;       // left border
    dX2=dX3=iWidth-iMargin;    // right border
    dY1=dY2=iMargin;           // top border
    dY3=dY4=iHeight-iMargin;   // lower border

    // #grid cells along distance and velocity -- auxiliary first, need number of WHOLE cells
    int iFromX=qRound(( dX1-dViewD0Pix)/dDGridStepPix+0.5e0);  // X1 is left border
    int iToX  =qRound(( dX2-dViewD0Pix)/dDGridStepPix-0.5e0);  // X2 is right border
    int iFromY=qRound((-dY3+dViewV0Pix)/dVGridStepPix+0.5e0);  // Y3 is lower border
    int iToY  =qRound((-dY1+dViewV0Pix)/dVGridStepPix-0.5e0);  // Y1 is upper border

    // Limitation on too dense grid
    int iGridMaxTicks=QTARGETSMAP_TICKS_OVERFLOW;
    // if inappropriate magnification then skip
    if (   (iToY-iFromY)<1
        || (iToX-iFromX)<1
        || (iToY-iFromY+1)>iGridMaxTicks
        || (iToX-iFromX+1)>iGridMaxTicks) {
        m_qsLastError = QString("Grid size illegal: %1 x %2").arg(iToX-iFromX).arg(iToY-iFromY);
        painter.restore();
        qDebug() << "m_qsLastError = " << m_qsLastError;
        return false;
    }

    // find the tick label with max width. If it fits not then quit
    int i;
    int iMaxWidthX,iSomeWidthX;
    int iMaxWidthY,iSomeWidthY;
    QString qsSomeLabel;
    qsSomeLabel=QString::number(iFromX*m_dDGridStep,'f',iPrecD);
    iMaxWidthX=fmTickLabel.width(qsSomeLabel);
    qsSomeLabel=QString::number(iFromY*m_dVGridStep,'f',iPrecV);
    iMaxWidthY=fmTickLabel.width(qsSomeLabel);
    for (i=iFromX; i<=iToX; i++) {
        qsSomeLabel=QString::number(i*m_dDGridStep,'f',iPrecD);
        iSomeWidthX=fmTickLabel.width(qsSomeLabel);
        iMaxWidthX=(iSomeWidthX>iMaxWidthX)?iSomeWidthX:iMaxWidthX;
    }
    for (i=iFromY; i<=iToY; i++) {
        qsSomeLabel=QString::number(i*m_dVGridStep,'f',iPrecV);
        iSomeWidthY=fmTickLabel.width(qsSomeLabel);
        iMaxWidthY=(iSomeWidthY>iMaxWidthY)?iSomeWidthY:iMaxWidthY;
    }

    // update ticks range with respect to lable width and height
    int iLabelHeight=fmTickLabel.height();
    iFromX = qRound(( dX1+iMaxWidthY+iTickLabelOffsetX-dViewD0Pix)   / dDGridStepPix + 0.5e0); // need number of WHOLE cells
    iToX   = qRound(( dX2-iMaxWidthY-iTickLabelOffsetX-dViewD0Pix)   / dDGridStepPix - 0.5e0); // right border
    iFromY = qRound((-dY4+iLabelHeight+iTickLabelOffsetY+dViewV0Pix) / dVGridStepPix + 0.5e0); // lower border
    iToY   = qRound((-dY1-iLabelHeight-iTickLabelOffsetY+dViewV0Pix) / dVGridStepPix - 0.5e0); // upper border
    if ( (iToX-iFromX)*iMaxWidthX > iWidth ) {
        m_qsLastError = QString("No room for X labels");
        painter.restore();
        qDebug() << "m_qsLastError = " << m_qsLastError;
        return false;
    }
    if ( (iToX-iFromX)<1 || (iToY-iFromY)<1 ) {
        m_qsLastError = QString("Grid size illegal: %1 x %2").arg(iToX-iFromX).arg(iToY-iFromY);
        painter.restore();
        qDebug() << "m_qsLastError = " << m_qsLastError;
        return false;
    }

    // if negative, shift origin of distance axis to left border
    if (iFromX < 0) {
        iToX -= iFromX;
        dViewD0Pix += iFromX * dDGridStepPix;
        m_dViewD0 = -dViewD0Pix / m_dScaleD ;
        iFromX = 0;
    }

    double dGridLeft   =dViewD0Pix + iFromX *dDGridStepPix;
    double dGridRight  =dViewD0Pix + iToX   *dDGridStepPix;
    double dGridBottom =dViewV0Pix - iFromY *dVGridStepPix;
    double dGridTop    =dViewV0Pix - iToY   *dVGridStepPix;

    // actual grid X axis
    double dXCur,dYCur;
    for (i=iFromX; i<=iToX; i++) {
        dXCur=dViewD0Pix+i*dDGridStepPix;
        if (dXCur<0 || dXCur>iWidth) continue;
        QString qsLabel=QString::number(i*m_dDGridStep,'f',iPrecD);
        int iLabelWidth=fmTickLabel.width(qsLabel);
        painter.drawText(dXCur-iLabelWidth/2,
                         dGridTop-iLabelHeight-iTickLabelOffsetY+fmTickLabel.ascent(),
                         qsLabel);
        painter.drawText(dXCur-iLabelWidth/2,
                         dGridBottom+iTickLabelOffsetY+fmTickLabel.ascent(),
                         qsLabel);
        if (i==0 || i==iFromX || i==iToX) {
            qpGridPen.setStyle(Qt::SolidLine);
            qpGridPen.setWidth(iAxisThickness);
        }
        else {
            qpGridPen.setStyle(Qt::DashLine);
            qpGridPen.setWidth(iGridThickness);
        }
        painter.setPen(qpGridPen);
        painter.drawLine(dXCur,dGridTop,dXCur,dGridBottom);
    }
    for (i=iFromY; i<=iToY; i++) {
        dYCur=dViewV0Pix-i*dVGridStepPix;
        if (dYCur<0 || dYCur>iHeight) continue;
        QString qsLabel=QString::number(i*m_dVGridStep,'f',iPrecV);
        int iLabelWidth=fmTickLabel.width(qsLabel);
        painter.drawText(dGridLeft-iTickLabelOffsetX-iLabelWidth,
                         dYCur-iLabelHeight/2.0+fmTickLabel.ascent(),
                         qsLabel);
        painter.drawText(dGridRight+iTickLabelOffsetX,
                         dYCur-iLabelHeight/2.0+fmTickLabel.ascent(),
                         qsLabel);
        if (i==0 || i==iFromY || i==iToY) {
            qpGridPen.setStyle(Qt::SolidLine);
            qpGridPen.setWidth(iAxisThickness);
        }
        else {
            qpGridPen.setStyle(Qt::DashLine);
            qpGridPen.setWidth(iGridThickness);
        }
        painter.setPen(qpGridPen);
        painter.drawLine(dGridLeft,dYCur,dGridRight,dYCur);
    }
    if (!m_pSafeParams) {
        m_pSafeParams = new struct GridSafeParams;
        m_pSafeParams->dViewD0    = m_dViewD0;
        m_pSafeParams->dViewV0    = m_dViewV0;
        m_pSafeParams->dScaleD    = m_dScaleD;
        m_pSafeParams->dScaleV    = m_dScaleV;
        m_pSafeParams->dDGridStep = m_dDGridStep;
        m_pSafeParams->dVGridStep = m_dVGridStep;
    }
    // pop painter settings from stack
    painter.restore();
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::zoomMap(MapWidget *pMapWidget, bool bZoomAlongD, bool bZoomAlongV, bool bZoomIn /* = true */ ) {
    // no operations in error state
    if (!m_qsLastError.isEmpty()) return;

    // widget center
    QRect qrWidgetSize = pMapWidget->geometry();
    int iXC = qrWidgetSize.width()/2;
    int iYC = qrWidgetSize.height()/2;
    // vector (phy coordinates) from widget center to physical coordinate origin
    double dVecX = -m_dViewD0 - iXC/m_dScaleD;
    double dVecY = -m_dViewV0 + iYC/m_dScaleV;

    double dMultiplier;
    double dLastVal;
    if (bZoomIn) {
        dMultiplier = 1.1;
    }
    else {
        dMultiplier = 1/1.1;
    }
    // distance zoom
    if (bZoomAlongD) {
        dLastVal = m_dScaleD;
        m_dScaleD *= dMultiplier;
        // protect from too large zoom
        if (m_dScaleD >= m_dMaxScaleD || m_dScaleD <= m_dMinScaleD) m_dScaleD = dLastVal;
        // use adaptive grid step
        if (m_bAdaptiveGridStep) {
            double dDGridStepPix=m_dDGridStep*m_dScaleD;
            int iWidgetWidth = qrWidgetSize.width();
            if (dDGridStepPix * QTARGETSMAP_MINTICKS > iWidgetWidth) {
                m_dDGridStep*=0.5e0;
            }
            else if (dDGridStepPix * QTARGETSMAP_MAXTICKS < iWidgetWidth) {
                m_dDGridStep*=2.0e0;
            }
        }
    }
    // velocity zoom
    if (bZoomAlongV) {
        dLastVal = m_dScaleV;
        m_dScaleV *= dMultiplier;
        // protect from too large zoom
        if (m_dScaleV >= m_dMaxScaleV || m_dScaleV <= m_dMinScaleV) m_dScaleV = dLastVal;
        // use adaptive grid step
        if (m_bAdaptiveGridStep) {
            double dVGridStepPix=m_dVGridStep*m_dScaleV;
            int iWidgetHeight = qrWidgetSize.height();
            if (dVGridStepPix * QTARGETSMAP_MINTICKS > iWidgetHeight) {
                m_dVGridStep*=0.5e0;
            }
            else if (dVGridStepPix * QTARGETSMAP_MAXTICKS < iWidgetHeight) {
                m_dVGridStep*=2.0e0;
            }
        }
    }

    // (dimensional) coordinate of widget (0,0) relative to physical coordinate origin, physical coordinate system
    m_dViewD0 = -iXC/m_dScaleD - dVecX;
    m_dViewV0 =  iYC/m_dScaleV - dVecY;

    // some QWidget shifts/rescales destroy last mouse location
    if (m_pMouseStill) {
        delete m_pMouseStill;
        m_pMouseStill = NULL;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::zoomMap(MapWidget *pMapWidget, int iX, int iY, bool bZoomIn /*= true*/) {
    // no operations in error state
    if (!m_qsLastError.isEmpty()) return;

    QRect qrWidgetSize = pMapWidget->geometry();
    // vector (phy coordinates) from mouse pointer to physical coordinate origin
    double dVecX = -m_dViewD0 - iX/m_dScaleD;
    double dVecY = -m_dViewV0 + iY/m_dScaleV;
    // scaling factor
    double dMultiplier;
    if (bZoomIn) {
        dMultiplier = 1.1;
    }
    else {
        dMultiplier = 1/1.1;
    }
    double dLastValD = m_dScaleD;
    double dLastValV = m_dScaleV;
    m_dScaleD *= dMultiplier; m_dScaleV *= dMultiplier;
    // protect from too large scale
    if (m_dScaleD >= m_dMaxScaleD || m_dScaleV >= m_dMaxScaleV
     || m_dScaleD <= m_dMinScaleD || m_dScaleV <= m_dMinScaleV) {
        m_dScaleD = dLastValD; m_dScaleV = dLastValV;
        return;
    }
    // adapt grid step size
    if (m_bAdaptiveGridStep) {
        double dDGridStepPix=m_dDGridStep*m_dScaleD;
        double dVGridStepPix=m_dVGridStep*m_dScaleV;
        int iWidgetWidth = qrWidgetSize.width();
        int iWidgetHeight = qrWidgetSize.height();
        // adapt distance grid
        if (dDGridStepPix * QTARGETSMAP_MINTICKS > iWidgetWidth) {
            m_dDGridStep*=0.5e0;
        }
        else if (dDGridStepPix * QTARGETSMAP_MAXTICKS < iWidgetWidth) {
            m_dDGridStep*=2.0e0;
        }
        // adapt velocity grid
        if (dVGridStepPix * QTARGETSMAP_MINTICKS > iWidgetHeight) {
            m_dVGridStep*=0.5e0;
        }
        else if (dVGridStepPix * QTARGETSMAP_MAXTICKS < iWidgetHeight) {
            m_dVGridStep*=2.0e0;
        }
    }

    // (dimensional) coordinate of widget (0,0) relative to physical coordinate origin, physical coordinate system
    m_dViewD0 = -iX/m_dScaleD - dVecX;
    m_dViewV0 =  iY/m_dScaleV - dVecY;

    // some QWidget shifts/rescales destroy last mouse location
    if (m_pMouseStill) {
        delete m_pMouseStill;
        m_pMouseStill = NULL;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::addTargetMarker(QTargetMarker* pTargetMarker) {
    if (pTargetMarker) m_qlTargets.append(pTargetMarker);
    emit doUpdate();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::clearMarkers() {
    qDebug() << "clearMarkers() called";
    while (m_qlTargets.count()) delete m_qlTargets.takeLast();
    emit doUpdate();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QTargetsMap::getFirstFormular(QPoint &qpPos) {
    m_pMovingFormular = NULL;
    for (int i=0; i < m_qlFormulars.size(); i++) {
        QFormular *pFormular = m_qlFormulars.at(i);
        if (pFormular->contains(qpPos)) m_pMovingFormular = pFormular;
    }
    return (m_pMovingFormular != NULL);
}
