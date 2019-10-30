#include "qtargetsmap.h"
#include "qinisettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>

QTargetsMap::QTargetsMap() : QObject(0) {
    [[maybe_unused]] QIniSettings &iniSettings = QIniSettings::getInstance();
    [[maybe_unused]] QIniSettings::STATUS_CODES scRes;
    // iniSettings.setDefault(SETTINGS_SOME_VALUE,"SoveValueDefault");
    // m_qsSomeValue = iniSettings.value(SETTINGS_SOME_VALUE,scRes).toString();
}

QTargetsMap::~QTargetsMap() {
    [[maybe_unused]] QIniSettings &iniSettings = QIniSettings::getInstance();
    // iniSettings.setValue(SETTINGS_SOME_VALUE, m_qsSomeValue);
}

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
    // qDebug() << "m_qsDBFile = " << m_qsDBFile;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MapWidget *QTargetsMap::getMapInstance() {
    return new MapWidget(this);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::mapPaintEvent(MapWidget *pMapWiget, [[maybe_unused]] QPaintEvent *qpeEvent) {
    QPainter painter;
    painter.begin(pMapWiget);
    painter.setRenderHint(QPainter::Antialiasing);

    // coordinate grid
    drawGrid(pMapWiget, painter);

    //showPosts(painter);
    //drawPoints(painter);

    /*
    QSize szIndicator = pMapWiget->size();
    double dXC=szIndicator.width()/2.0e0;
    double dYC=szIndicator.height()/2.0e0;
    QPen qpPostLabel(Qt::red);
    painter.setPen(qpPostLabel);
    QFont font=QApplication::font();
    font.setWeight(QFont::Bold);
    font.setPointSize(32);
    painter.setFont(font);
    QFontMetrics fmPostLabel(font,pMapWiget);
    QString qsLabel("Ahahaha");
    painter.drawText(dXC,dYC,qsLabel);
    painter.end();
    */

    painter.end();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTargetsMap::drawGrid(MapWidget *pMapWiget, QPainter &painter) {
    //------------------------------------------------------------------------
    // all coordinates declared here are in widget coordinate system
    // i.e. (0,0) is left upper corner
    //------------------------------------------------------------------------
    double dXC,dYC; // view widget center
    double dX0,dY0; // coordinate origin (pix) in widget coordinates

    double dX1,dY1, // left upper
           dX2,dY2, // right upper
           dX3,dY3, // right lower
           dX4,dY4;  // left lower

    // some constants
    int iHeight = pMapWiget->height();
    int iWidth  = pMapWiget->width();
    dXC = iWidth/2.0e0;  // Indicator center in screen coordinates
    dYC = iHeight/2.0e0; // Indicator center in screen coordinates
    int iMargin = 2;
    int iLeftMargin = 2;
    static double dScaleV=2.0e0; // pixels per m/s
    static double dScaleD=2.0e0; // pixels per meter
    QColor qcGridColor(Qt::darkGray);
    // grid steps, dimensional
    double dVGridStep=20.0e0; // m/s
    double dDGridStep=20.0e0;  // m
    // grid steps in pixels
    double dVGridStepPix=dVGridStep*dScaleV;
    double dDGridStepPix=dDGridStep*dScaleD;
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
    QFontMetrics fmTickLabel(font,pMapWiget);

    // coordiante origin
    QString qsExample = QString::number(-dVGridStep*(dYC/dVGridStepPix),'f',0);
    int iLabelMaxWidth = fmTickLabel.width(qsExample);  // typical label with
    int iRightmostVAxisShiftPix = iLabelMaxWidth + iTickLabelOffsetX; // Correction to axis position due to labels
    double dViewD0Pixels = dXC-iLeftMargin-iRightmostVAxisShiftPix; // Indicator center on the distance axis in pixels
    double dViewD0Pix = qFloor(dViewD0Pixels / dDGridStepPix) * dDGridStepPix; // Position of indicator center relative to coordinate origin in pixels
    // double dViewD0 = dViewD0Pix * dScaleD;  // Indicator center on distance axis m
    double dViewV0=0.0e0; // Indicator center on the velocity axis m/s
    double dViewV0Pix = dViewV0*dScaleV; // Position of indicator center relative to coordinate origin in pixels

    // Limitation on too dense grid
    int iGridMaxTicks=200;

    // view position within Widget
    dX0 = dXC - dViewD0Pix; // Coordinate origin in screen coordinates (distance origin)
    dY0 = dYC + dViewV0Pix; // Coordinate origin in screen coordinates (velocity)
    dX1=dX4=iLeftMargin;
    dX2=dX3=iWidth-iMargin;
    dY1=dY2=iMargin;
    dY3=dY4=iHeight-iMargin;

    // #grid cells along distance and velocity -- auxiliary first
    int iFromX=qRound((dX1-dX0)/dDGridStepPix+0.5e0);  // need number of WHOLE cells
    int iToX  =qRound((dX2-dX0)/dDGridStepPix-0.5e0);
    int iFromY=qRound((dY1-dY0)/dVGridStepPix+0.5e0);
    int iToY  =qRound((dY3-dY0)/dVGridStepPix-0.5e0);

    // colors and fonts
    double dXCur,dYCur;
    QBrush qbGridBrush(qcGridColor);
    QPen qpGridPen(qbGridBrush,iGridThickness,Qt::SolidLine,Qt::RoundCap);
    painter.setPen(qpGridPen);

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
    iFromX = qRound(( dX1+iMaxWidthY+iTickLabelOffsetX-dX0)   / dDGridStepPix + 0.5e0); // need number of WHOLE cells
    iToX   = qRound(( dX2-iMaxWidthY-iTickLabelOffsetX-dX0)   / dDGridStepPix - 0.5e0);
    iFromY = qRound((-dY4+iLabelHeight+iTickLabelOffsetY+dY0) / dVGridStepPix + 0.5e0);
    iToY   = qRound((-dY1-iLabelHeight-iTickLabelOffsetY+dY0) / dVGridStepPix - 0.5e0);
    if ((iToX-iFromX+1)*iMaxWidthX+iLeftMargin+iMargin+2*iTickLabelOffsetX > iWidth) return;

    double dGridLeft   =dX0 + iFromX *dDGridStepPix;
    double dGridRight  =dX0 + iToX   *dDGridStepPix;
    double dGridBottom =dY0 - iFromY *dVGridStepPix;
    double dGridTop    =dY0 - iToY   *dVGridStepPix;

    // if inappropriate magnification then skip
    if (   (iToY-iFromY)<1
        || (iToX-iFromX)<1
        || (iToX-iFromX+1)>iGridMaxTicks
        || (iToY-iFromY+1)>iGridMaxTicks ) return;

    // actual grid X axis
    for (i=iFromX; i<=iToX; i++) {
        dXCur=dX0+i*dDGridStepPix;
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
        dYCur=dY0-i*dVGridStepPix;
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
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MapWidget::MapWidget(QTargetsMap *owner, QWidget *parent /* =0 */)
      : QOpenGLWidget(parent)
      , m_pOwner(owner) {
    setAutoFillBackground(false);
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
