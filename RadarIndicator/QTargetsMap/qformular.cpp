#include "qformular.h"
#include "qtargetsmap.h"

const quint64 QFormular::m_uFormularLifetime = 10000;
const quint64 QFormular::m_uFormularCutoffPix = 10;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QFormular::QFormular(struct sMouseStillPos *pMouseStill,
                     // scales over horizontal and vertical direction
                     double dScaleD, // pixels per m
                     double dScaleV, // pixels per m/s
                     // the only reference values are coordinates of widget center along dimensional axes
                     double dViewD0, // along distance axis (m)
                     double dViewV0, // along velocity axis (m/s)
                     QObject *parent /* = 0 */ )
               : QObject(parent)
               , m_dScaleD(dScaleD)
               , m_dScaleV(dScaleV)
               , m_dViewD0(dViewD0)
               , m_dViewV0(dViewV0)
               , m_pTargetMarker(NULL)
               , m_bStale(false) {
    Q_ASSERT(pMouseStill != 0);
    m_mouseStill = *pMouseStill;
    // some constants
    int iHeight = m_mouseStill.geometry.height();
    int iWidth  = m_mouseStill.geometry.width();
    double dXC,dYC; // view widget center
    dXC = iWidth/2.0e0;  // Indicator center in screen coordinates
    dYC = iHeight/2.0e0; // Indicator center in screen coordinates
    // Position of physical coordinate origin in pixels (view coordinates)
    double dViewD0Pix = dXC - dViewD0 * dScaleD;
    double dViewV0Pix = dYC + dViewV0 * dScaleV;
    // Pysical coordinates of mouse pointer
    Q_ASSERT(dScaleD != 0);
    Q_ASSERT(dScaleV != 0);
    m_dMouseDPhys = ( m_mouseStill.pos.x() - dViewD0Pix)/dScaleD;
    m_dMouseVPhys = (-m_mouseStill.pos.y() + dViewV0Pix)/dScaleV;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QFormular::isStale() {
    if (m_bStale) return true;
    return (m_mouseStill.last.msecsTo(QTime::currentTime()) > m_uFormularLifetime);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFormular::setStale() {
    // ensure this formular is no more used
    m_bStale = true;
    // m_pTargetMarker: object was destroyed
    m_pTargetMarker = NULL;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QFormular::selectTarg(QList<QTargetMarker*> qlTargets) {
    QList<double> qlDists;
    QList<double> qlIndxs;
    // if currently no targets detected - then skip
    if (!qlTargets.size()) return false;
    // calculate all dists (in pixels) from targets to (m_dMouseDPhys,m_dMouseVPhys)
    for (int i=0; i<qlTargets.size(); i++) {
        double xx = qlTargets.at(i)->x()-m_dMouseDPhys;
        double yy = qlTargets.at(i)->y()-m_dMouseVPhys;
        xx *= m_dScaleD;
        yy *= m_dScaleV;
        qlDists << abs(xx)+abs(yy);
        qlIndxs << i;
    }
    // sort the distances
    for (int i=0; i<qlTargets.size()-1; i++) {
        for (int j=i+1; j<qlTargets.size(); j++) {
            if (qlDists.at(i) > qlDists.at(j)) {
                qlDists.swap(i,j);
                qlTargets.swap(i,j);
                qlIndxs.swap(i,j);
            }
        }
    }
    // smallest distance is too many pixels - skip
    for (int i=0; i<qlDists.size(); i++) {
        if (qlDists.at(i) > m_uFormularCutoffPix) return false;
        QTargetMarker *pTargetMarker = qlTargets.at(i);
        if (pTargetMarker->hasFormular()) continue;
        pTargetMarker->setFormular(this);
        m_pTargetMarker = pTargetMarker;
        return true;
    }
    return false;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFormular::drawFormular(MapWidget *pMapWidget,
                             // scales over horizontal and vertical direction
                             double dScaleD, // pixels per m
                             double dScaleV, // pixels per m/s
                             // the only reference values are coordinates of widget center along dimensional axes
                             double dViewD0, // along distance axis (m)
                             double dViewV0, // along velocity axis (m/s)
                             QPainter &painter) {
    // check pointer valid (paranoia?)
    if (!pMapWidget) return;
    // if formular is stale then m_pTargetMarker no longer exists
    if (m_bStale) return;
    // if no target marker was found close to mouse pointer - then skip
    if (!m_pTargetMarker) return;
    // save painter state
    // painter.save();
    int iHeight = pMapWidget->height();
    int iWidth  = pMapWidget->width();
    // view widget center
    double dXC,dYC; // widget coordinate system
    dXC = iWidth/2.0e0;  // Indicator center in screen coordinates
    dYC = iHeight/2.0e0; // Indicator center in screen coordinates

    // refresh current scales and origin shift
    m_dScaleD = dScaleD;
    m_dScaleV = dScaleV;
    m_dViewD0 = dViewD0;
    m_dViewV0 = dViewV0;

    // Position of physical coordinate origin in pixels (view coordinates)
    double dViewD0Pix = dXC - m_dViewD0 * m_dScaleD;
    double dViewV0Pix = dYC + m_dViewV0 * m_dScaleV;

    // Position of target in pixels (view coordinates)
    double dTarDPhys = m_pTargetMarker->x();
    double dTarVPhys = m_pTargetMarker->y();
    double dTarDPix = dViewD0Pix + dTarDPhys * m_dScaleD;
    double dTarVPix = dViewV0Pix - dTarVPhys * m_dScaleV;
    // target outside widget area - skip
    if (dTarDPix > iWidth || dTarDPix <0 || dTarVPix<0 || dTarVPix>iHeight) return;

    // Draw the formular
    QPointF qpTar(dTarDPix,dTarVPix);
    QRectF qrBounding(0.0,0.0,iWidth,iHeight);
    if (!qrBounding.contains(qpTar)) {
        // painter.restore();
        return;
    }
    // Background color
    QColor qcFormularBg(Qt::red);
    qcFormularBg.setAlpha(128);
    QBrush qbFormularBrush(qcFormularBg);
    // Border color
    QBrush qbFormularBorder(Qt::darkGray,Qt::SolidPattern);
    int iFormularThickness=2;
    QPen qpFormularPen(qbFormularBorder,iFormularThickness,Qt::SolidLine,Qt::RoundCap);
    // font metrics
    int iLegendSize = 14;
    QFont font=QApplication::font();
    font.setWeight(QFont::Normal);
    font.setPointSize(iLegendSize);
    painter.setFont(font);
    QFontMetrics fmTickLabel(font,pMapWidget);
    // formular rectangle
    QPoint qpFormularOffset = QPoint(30.,30.);
    QString qsMesg = m_pTargetMarker->mesgString();
    QRect qrTxt = fmTickLabel.boundingRect(qsMesg);
    QRectF qrFormuRect = QRectF(0.0, 0.0, qrTxt.width()+6, qrTxt.height());
    qrFormuRect.moveTo(qpFormularOffset + qpTar);
    if (qrFormuRect.top() < 0.) qrFormuRect.translate(0.,-qrFormuRect.top());
    if (qrFormuRect.left() < 0.) qrFormuRect.translate(-qrFormuRect.left(),0.);
    if (qrFormuRect.bottom() > qrBounding.height()) qrFormuRect.translate(QPointF(0.,qrBounding.height()-qrFormuRect.bottom()));
    if (qrFormuRect.right() > qrBounding.width()) qrFormuRect.translate(QPointF(qrBounding.width()-qrFormuRect.right(),0.));
    QPointF pt0(qrFormuRect.left(),qrFormuRect.top());
    double d0 = _hypot(qpTar.x()-pt0.x(),qpTar.y()-pt0.y());
    QPointF pt1(qrFormuRect.right(),qrFormuRect.top());
    double d1 = _hypot(qpTar.x()-pt1.x(),qpTar.y()-pt1.y());
    QPointF pt2(qrFormuRect.right(),qrFormuRect.bottom());
    double d2 = _hypot(qpTar.x()-pt2.x(),qpTar.y()-pt2.y());
    QPointF pt3(qrFormuRect.left(),qrFormuRect.bottom());
    double d3 = _hypot(qpTar.x()-pt3.x(),qpTar.y()-pt3.y());
    if (d1 < d0) {
        d0 = d1; pt0 = pt1;
    }
    if (d2 < d0) {
        d0 = d2; pt0 = pt2;
    }
    if (d3 < d0) {
        d0 = d3; pt0 = pt3;
    }
    painter.drawLine(qpTar,pt0);
    QPointF qpTxt(qrFormuRect.left(),qrFormuRect.top());
    qpTxt += QPointF(3.0,qrTxt.height()-3.0);
    // start actual grawing
    painter.setPen(qpFormularPen);
    painter.setBrush(qbFormularBrush);
    painter.drawRect(qrFormuRect);
    painter.drawText(qpTxt,qsMesg);
    // restore painter state
    // painter.restore();
}
