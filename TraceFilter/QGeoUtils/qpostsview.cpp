#include "qpostsview.h"
#include "tdcord.h"

#define RAD_2_DEG 57.29577951308232
#define DEG_2_RAD (.0174532925199432958)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPostsView::QPostsView(QString qsMainctrlCfg, QWidget *parent /* = 0 */)
      : QWidget(parent, Qt::WindowFlags())
      , m_iLabelSize(16)
      , m_iPenWidth(4)
      , m_iMargin(5)
      , m_iWgtSz(80)
      , m_iOffset(3)
      , m_pMainCtrl(NULL) {
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    onMainctrlChanged(qsMainctrlCfg);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPostsView::onMainctrlChanged(QString qsMainctrlCfg) {
    bool bRetVal=false;
    m_qlPosts.clear();
    m_qlPostIds.clear();
    m_qfMainCtrl.setFileName(qsMainctrlCfg);
    PMAINCTRL pMainCtrl=m_pMainCtrl=NULL;
    if (m_qfMainCtrl.open(QIODevice::ReadOnly)) {
        pMainCtrl=(PMAINCTRL)m_qfMainCtrl.map(0,m_qfMainCtrl.size());
        m_qfMainCtrl.close();
        BLH blhViewPoint;
        if (pMainCtrl && TdCord::getViewPoint(&blhViewPoint,pMainCtrl)) { // radians,meters
            QList<XYZ> qlTcPosts;
            QList<int> qlPostIds;
            if (TdCord::getTopocentricPostsList(&blhViewPoint,qlTcPosts,qlPostIds,pMainCtrl)) {
                if (qlTcPosts.count()==4 && qlPostIds.count()==4) {
                    for (int i=0; i<qlTcPosts.count(); i++) {
                        XYZ xyzTcPost = qlTcPosts.at(i);
                        m_qlPosts << QPoint(xyzTcPost.dX,xyzTcPost.dY);
                        m_qlPostIds << qlPostIds.at(i);
                    }
                    m_qsMainctrlCfg = qsMainctrlCfg;
                    m_pMainCtrl = pMainCtrl;
                    m_blhViewPont = blhViewPoint;
                    bRetVal=true;
                }
            }
        }
    }
    repaint();
    return bRetVal;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void QPostsView::paintEvent(QPaintEvent *qpeEvent) {
    QPainter painter;
    painter.begin(this);
    QBrush qbrBg(Qt::white,Qt::SolidPattern);
    QSize szWgt=size();
    painter.fillRect(0,0,szWgt.width(),szWgt.height(),qbrBg);
    QPen qpLbl(Qt::black);
    painter.setPen(qpLbl);
    QFont font=QApplication::font();
    font.setPointSize(m_iLabelSize);
    painter.setFont(font);
    if (m_qlPostIds.count()!=4 || m_qlPosts.count()!=4) {
        painter.drawText(QRect(0,0,szWgt.width(),szWgt.height()),Qt::AlignCenter,"Error");
        painter.end();
        return;
    }
    QFontMetrics fmTickLabel(font);
    QPen qpenPost(Qt::black,m_iPenWidth,Qt::SolidLine,Qt::RoundCap);
    // determine scale
    int iXmin=1.0e10, iXmax=-1.0e10, iYmin=1.0e10, iYmax=-1.0e10;
    int iLblWidth=0;
    int iLblHeight=0;
    for (int i=0; i<m_qlPosts.count(); i++) {
        int iX=m_qlPosts.at(i).x();
        int iY=m_qlPosts.at(i).y();
        iXmin=(iXmin>iX)?iX:iXmin;
        iXmax=(iXmax<iX)?iX:iXmax;
        iYmin=(iYmin>iY)?iY:iYmin;
        iYmax=(iYmax<iY)?iY:iYmax;

        QRect qrLbl = fmTickLabel.tightBoundingRect(QString::number(m_qlPostIds.at(i)));
        int iWidth = qrLbl.width();
        int iHeight = qrLbl.height();
        iLblWidth = (iWidth>iLblWidth)?iWidth:iLblWidth;
        iLblHeight = (iHeight>iLblHeight)?iHeight:iLblHeight;
    }
    if (iXmin>iXmax || iYmin > iYmax) {
        painter.drawText(QRect(0,0,szWgt.width(),szWgt.height()),Qt::AlignCenter,"Error");
        painter.end();
        return;
    }
    double dPhyWidth = iXmax - iXmin;
    double dPhyHeight = iYmax - iYmin;
    double dScrWidth = szWgt.width() - 2*m_iMargin - iLblWidth - m_iOffset - m_iPenWidth;
    double dScrHeight = szWgt.height() - 2*m_iMargin - iLblHeight - m_iOffset - m_iPenWidth;
    double dScaleX=dScrWidth/dPhyWidth;
    double dScaleY=dScrHeight/dPhyHeight;
    if (dScaleX < 1.e-6 || dScaleY < 1.0e-6) {
        painter.drawText(QRect(0,0,szWgt.width(),szWgt.height()),Qt::AlignCenter,"Error");
        painter.end();
        return;
    }
    int iScrX0=m_iMargin+iLblWidth+m_iOffset;
    int iScrY0=m_iMargin+iLblHeight+m_iOffset;
    for (int i=0; i<m_qlPosts.count(); i++) {
        int iPostId = m_qlPostIds.at(i);
        QPoint qpPost = m_qlPosts.at(i);
        QString qsPostId = QString::number(iPostId);
        int iPtX = iScrX0 + (qpPost.x()-iXmin)*dScaleX;
        int iPtY = iScrY0 + (iYmax - qpPost.y())*dScaleY;
        painter.setPen(qpenPost);
        painter.drawPoint(iPtX,iPtY);
        painter.setPen(qpLbl);
        painter.drawText(iPtX-m_iOffset-iLblWidth,iPtY-m_iOffset,qsPostId);
    }
    painter.end();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ QSize  QPostsView::sizeHint() const {
    return QSize(m_iWgtSz,m_iWgtSz);
}

