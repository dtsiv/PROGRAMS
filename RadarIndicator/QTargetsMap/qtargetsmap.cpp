#include "qtargetsmap.h"
#include "qinisettings.h"
#include "qexceptiondialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>

// time delay to show formular
const quint64 QTargetsMap::m_iFormularDelay = 1000;

QTargetsMap::QTargetsMap(QWidget * pOwner /* = 0 */)
         : QObject(0)
         , m_dScaleD(2.0e0)
         , m_dMaxScaleD(10.0e0)
         , m_dScaleV(2.0e0)
         , m_dMaxScaleV(10.0e0)
         , m_dViewD0(100)
         , m_dViewV0(0)
         , m_pSafeParams(NULL)
         , m_pOwner(pOwner)
         , m_pMouseStill(NULL) {
    [[maybe_unused]] QIniSettings &iniSettings = QIniSettings::getInstance();
    [[maybe_unused]] QIniSettings::STATUS_CODES scRes;
    m_qlTargets << new QTargetMarker(200,0,"SU2134");
    m_qlTargets << new QTargetMarker(150,20,"IR011");
    m_qlTargets << new QTargetMarker(350,-20,"IRA111");
    // iniSettings.setDefault(SETTINGS_SOME_VALUE,"SoveValueDefault");
    // m_qsSomeValue = iniSettings.value(SETTINGS_SOME_VALUE,scRes).toString();
    QObject::connect(&m_qtFormular,SIGNAL(timeout()),SLOT(onFormularTimeout()));
    m_qtFormular.start(m_iFormularDelay);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QTargetsMap::~QTargetsMap() {
    if (m_pSafeParams) {
        delete m_pSafeParams;
        m_pSafeParams = 0;
    }
    [[maybe_unused]] QIniSettings &iniSettings = QIniSettings::getInstance();
    // iniSettings.setValue(SETTINGS_SOME_VALUE, m_qsSomeValue);
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
    QFormular *pFormular = new QFormular(m_pMouseStill,m_dScaleD,m_dScaleV,m_dViewD0,m_dViewV0);
    if (!pFormular->selectTarg(m_qlTargets)) { delete pFormular; return; }
    m_qlFormulars.append(pFormular);
    delete m_pMouseStill;
    m_pMouseStill = 0;
    emit doUpdate();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    [[maybe_unused]] QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    [[maybe_unused]] QHBoxLayout *pHLayout=new QHBoxLayout;
    pHLayout->addWidget(new QLabel("Grid settings here..."));
    QVBoxLayout *pVLayout=new QVBoxLayout;
    pVLayout->addLayout(pHLayout);
    pVLayout->addStretch();
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QTARGETSMAP_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::propChanged(QObject *pPropDlg) {
    [[maybe_unused]] QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    // m_qsSomeValue = pPropPages->m_pleSomeValue->text();
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
    if (m_pSafeParams) {
        m_dViewD0 = m_pSafeParams->dViewD0;
        m_dViewV0 = m_pSafeParams->dViewV0;
        m_dScaleD = m_pSafeParams->dScaleD;
        m_dScaleV = m_pSafeParams->dScaleV;
        // clear last error message
        m_qsLastError.clear();
        emit doUpdate();
    }
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
void QTargetsMap::resetScale() {
    if (m_pSafeParams) {
        m_dScaleD = m_pSafeParams->dScaleD;
        m_dScaleV = m_pSafeParams->dScaleV;
        m_dViewD0 = m_pSafeParams->dViewD0;
        m_dViewV0 = m_pSafeParams->dViewV0;
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

    // coordinate grid
    if (m_qsLastError.isEmpty()) {
        if (!drawGrid(pMapWidget, painter)) {
            if (!m_pSafeParams) {
                m_qsLastError = "Grid safe parameters missing";
            }
            QExceptionDialog *pDlg = new QExceptionDialog(m_qsLastError, pMapWidget);
            QObject::connect(pDlg,SIGNAL(accepted()),SLOT(onExceptionDialogClosed()));
            pDlg -> setAttribute(Qt::WA_DeleteOnClose);
            pDlg -> open();
        }
        for (int i=0; i<m_qlFormulars.size(); i++ ) {
            m_qlFormulars.at(i)->drawFormular(pMapWidget,m_dScaleD,m_dScaleV,m_dViewD0,m_dViewV0,painter);
        }
    }
    painter.end();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QTargetsMap::drawGrid(MapWidget *pMapWidget, QPainter &painter) {
    //------------------------------------------------------------------------
    // all coordinates declared here are in widget coordinate system
    // i.e. (0,0) is left upper corner
    //------------------------------------------------------------------------
    double dX1,dY1, // left upper
           dX2,dY2, // right upper
           dX3,dY3, // right lower
           dX4,dY4; // left lower

    // some constants
    int iHeight = pMapWidget->height();
    int iWidth  = pMapWidget->width();
    double dXC,dYC; // view widget center
    dXC = iWidth/2.0e0;  // Indicator center in screen coordinates
    dYC = iHeight/2.0e0; // Indicator center in screen coordinates
    int iMargin = 2;
    int iLeftMargin = 2;
    QColor qcGridColor(Qt::darkGray);
    // grid steps, dimensional
    double dVGridStep=20.0e0; // m/s
    double dDGridStep=20.0e0;  // m
    // grid steps in pixels
    double dVGridStepPix=dVGridStep*m_dScaleV;
    double dDGridStepPix=dDGridStep*m_dScaleD;
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
    double dViewD0Pix = dXC - m_dViewD0 * m_dScaleD;
    double dViewV0Pix = dYC + m_dViewV0 * m_dScaleV;

    // screen corners in widget coordinates
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
    int iGridMaxTicks=200;
    // if inappropriate magnification then skip
    if (   (iToY-iFromY)<1
        || (iToX-iFromX)<1
        || (iToY-iFromY+1)>iGridMaxTicks
        || (iToX-iFromX+1)>iGridMaxTicks) {
        m_qsLastError = QString("Grid size illegal: %1 x %2").arg(iToX-iFromX).arg(iToY-iFromY);
        return false;
    }

    // find the tick label with max width. If it fits not then quit
    int i;
    int iMaxWidthX,iSomeWidthX;
    int iMaxWidthY,iSomeWidthY;
    QString qsSomeLabel;
    qsSomeLabel=QString::number(iFromX*dDGridStep,'f',0);
    iMaxWidthX=fmTickLabel.width(qsSomeLabel);
    qsSomeLabel=QString::number(iFromY*dVGridStep,'f',0);
    iMaxWidthY=fmTickLabel.width(qsSomeLabel);
    for (i=iFromX; i<=iToX; i++) {
        qsSomeLabel=QString::number(i*dDGridStep,'f',0);
        iSomeWidthX=fmTickLabel.width(qsSomeLabel);
        iMaxWidthX=(iSomeWidthX>iMaxWidthX)?iSomeWidthX:iMaxWidthX;
    }
    for (i=iFromY; i<=iToY; i++) {
        qsSomeLabel=QString::number(i*dVGridStep,'f',0);
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
        return false;
    }
    if ( (iToX-iFromX)<1 || (iToY-iFromY)<1 ) {
        m_qsLastError = QString("Grid size illegal: %1 x %2").arg(iToX-iFromX).arg(iToY-iFromY);
        return false;
    }

    // if negative, shift origin of distance axis to left border
    if (iFromX < 0) {
        iToX -= iFromX;
        dViewD0Pix += iFromX*dDGridStepPix;
        m_dViewD0 = (dXC - dViewD0Pix) / m_dScaleD;
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
        QString qsLabel=QString::number(i*dDGridStep,'f',0);
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
        QString qsLabel=QString::number(i*dVGridStep,'f',0);
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
        m_pSafeParams->dViewD0 = m_dViewD0;
        m_pSafeParams->dViewV0 = m_dViewV0;
        m_pSafeParams->dScaleD = m_dScaleD;
        m_pSafeParams->dScaleV = m_dScaleV;
    }
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::zoomMap(bool bZoomAlongD, bool bZoomAlongV, bool bZoomIn /* = true */ ) {
    // no operations in error state
    if (!m_qsLastError.isEmpty()) return;

    double dMultiplier;
    double dLastVal;
    if (bZoomIn) {
        dMultiplier = 1.1;
    }
    else {
        dMultiplier = 1/1.1;
    }
    if (bZoomAlongD) {
        dLastVal = m_dScaleD;
        m_dScaleD *= dMultiplier;
        if (m_dScaleD >= m_dMaxScaleD) m_dScaleD = dLastVal;
    }
    if (bZoomAlongV) {
        dLastVal = m_dScaleV;
        m_dScaleV *= dMultiplier;
        if (m_dScaleV >= m_dMaxScaleV) m_dScaleV = dLastVal;
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
void QTargetsMap::zoomMap(MapWidget *pMapWidget, int iX, int iY, bool bZoomIn /*= true*/) {
    // no operations in error state
    if (!m_qsLastError.isEmpty()) return;
    // position of widget center (view coordinates)
    double dXC = pMapWidget->width()/2.0e0;
    double dYC = pMapWidget->height()/2.0e0;
    // Position of physical coordinate origin in pixels (view coordinates)
    double dViewD0Pix = dXC - m_dViewD0 * m_dScaleD;
    double dViewV0Pix = dYC + m_dViewV0 * m_dScaleV;
    // vector (in pixels) from mouse location to physical coordinate origin
    double dVecX = dViewD0Pix - iX;
    double dVecY = dViewV0Pix - iY;
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
    if (m_dScaleD >= m_dMaxScaleD || m_dScaleV >= m_dMaxScaleV || abs(m_dScaleD)+abs(m_dScaleV) < 1.0e-8) {
        m_dScaleD = dLastValD; m_dScaleV = dLastValV;
        return;
    }
    // scaling factor
    dViewD0Pix = iX + dVecX*dMultiplier;
    dViewV0Pix = iY + dVecY*dMultiplier;
    // physical coordinate origin (dimensional)
    m_dViewD0 = ( dXC - dViewD0Pix) / m_dScaleD;
    m_dViewV0 = (-dYC + dViewV0Pix) / m_dScaleV;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MapWidget::MapWidget(QTargetsMap *owner, QWidget *parent /* =0 */)
      : QOpenGLWidget(parent)
      , m_bMapDragging(false)
      , m_pOwner(owner) {
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    setMinimumWidth(200);
    setMinimumHeight(200);
    setMouseTracking(true);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MapWidget::~MapWidget() {
    // Make sure the context is current and then explicitly
    // destroy all underlying OpenGL resources.
    // makeCurrent();
    // delete m_texture; delete m_shader; delete m_program;
    // m_vbo.destroy(); m_vao.destroy();
    // doneCurrent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MapWidget::paintEvent(QPaintEvent *qpeEvent) {
    m_pOwner->mapPaintEvent(this, qpeEvent);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MapWidget::initializeGL() {
          // Set up the rendering context, load shaders and other resources, etc.:
          QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
          f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
          // ...
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MapWidget::paintGL() {
          // Draw the scene:
          QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
          f->glClear(GL_COLOR_BUFFER_BIT);
          // ...
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MapWidget::resizeGL([[maybe_unused]]int w, [[maybe_unused]]int h) {
    // !!!doesn't work:  glViewport(0, 0, w/2, h/2);
    // glViewport() should instead appear in paintGL
    // ...
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void MapWidget::mousePressEvent(QMouseEvent *pe) {
    if (pe->button() == Qt::LeftButton) {
        m_qpLastPoint = pe->pos();
        m_bMapDragging = true;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void MapWidget::mouseMoveEvent(QMouseEvent *pe) {
    if ((pe->buttons() & Qt::LeftButton) && m_bMapDragging) {
        QPoint qpDelta = pe->pos() - m_qpLastPoint;
        m_qpLastPoint = pe->pos();
        m_pOwner->shiftView(qpDelta);
        repaint();
        return;
    }
    // mark mouse last position
    if (!m_pOwner->m_pMouseStill) m_pOwner->m_pMouseStill = new struct QFormular::sMouseStillPos;
    m_pOwner->m_pMouseStill->geometry = geometry();
    m_pOwner->m_pMouseStill->pos = pe->pos();
    m_pOwner->m_pMouseStill->last = QTime::currentTime();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void MapWidget::mouseReleaseEvent(QMouseEvent *pe) {
    if (pe->button() == Qt::LeftButton && m_bMapDragging) {
        m_bMapDragging = false;
        QPoint qpDelta = pe->pos() - m_qpLastPoint;
        m_pOwner->shiftView(qpDelta);
        repaint();
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void MapWidget::wheelEvent(QWheelEvent *pe) {
    if (pe->modifiers() & Qt::ControlModifier) {
        int iNumSteps = pe->delta();
        bool bMagnify = (iNumSteps>0);
        if (pe->modifiers() & Qt::ShiftModifier) {
            [[maybe_unused]]int iEventX = pe->x();
            [[maybe_unused]]int iEventY = pe->y();
            m_pOwner->zoomMap(this, iEventX, iEventY, bMagnify);
            repaint();
            return;
        }
        bool bZoomAlongD = true;
        bool bZoomAlongV = true;
        m_pOwner->zoomMap(bZoomAlongD, bZoomAlongV, bMagnify);
        repaint();
        return;
    }
    return;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void MapWidget::resizeEvent([[maybe_unused]]QResizeEvent *event) {
    QOpenGLWidget::resizeEvent(event);
    if (m_pOwner->m_pMouseStill) {
        delete m_pOwner->m_pMouseStill;
        m_pOwner->m_pMouseStill = NULL;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void MapWidget::keyReleaseEvent(QKeyEvent *pe) {
    if (     pe->key()==Qt::Key_Up
          || pe->key()==Qt::Key_Right
          || pe->key()==Qt::Key_Down
          || pe->key()==Qt::Key_Left) {
        switch (pe->key()) {
            case Qt::Key_Up:
                m_pOwner->shiftView(/*bool bVertical=*/true, /*bool bPositive=*/true); break;
            case Qt::Key_Down:
                m_pOwner->shiftView(/*bool bVertical=*/true, /*bool bPositive=*/false); break;
            case Qt::Key_Right:
                m_pOwner->shiftView(/*bool bVertical=*/false, /*bool bPositive=*/true); break;
            case Qt::Key_Left:
                m_pOwner->shiftView(/*bool bVertical=*/false, /*bool bPositive=*/false); break;
            default:
                // allow all other events
                QOpenGLWidget::keyPressEvent(pe);
                return;
        }
        repaint();
        return;
    }
    else if ((pe->key()==Qt::Key_Plus || pe->key()==Qt::Key_Minus
           || pe->key()==Qt::Key_Equal|| pe->key()==Qt::Key_Bar
           || pe->key()==Qt::Key_Underscore)
           && ((pe->modifiers() & Qt::ControlModifier)
           ||  (pe->modifiers() & Qt::ShiftModifier) )) {
        Qt::KeyboardModifiers kbmZoomAlong = pe->modifiers();
        int iKey = pe->key();
        bool bZoomAlongD = (kbmZoomAlong & Qt::ShiftModifier);
        bool bZoomIn = ((iKey==Qt::Key_Plus) || (iKey==Qt::Key_Equal));
        m_pOwner->zoomMap(bZoomAlongD, !bZoomAlongD, bZoomIn);
        repaint();
        return;
    }
    else if ( pe->key()==Qt::Key_0 && pe->modifiers() & Qt::ControlModifier) {
        m_pOwner->resetScale();
        repaint();
        return;
    }
    // allow all other events
    QOpenGLWidget::keyPressEvent(pe);
    return;
}
