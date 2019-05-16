#include "qpoimodel.h"
#include "qindicator.h"
#include "rmoexception.h"
#include "qvoiprocessor.h"
#include <QtGlobal>
#include <QKeyEvent>

struct LegendSettings QIndicator::m_lsLegend;
QRect QIndicator::m_qrIndGeometry;
double QIndicator::m_dScale = 1.0e-3;
double QIndicator::m_dDefaultScale = 1.0e-3;
double QIndicator::m_dScaleMin = 1.0e-6;
double QIndicator::m_dScaleMax = 1.0e0;
double QIndicator::m_dViewX0 = 0.0e0;
double QIndicator::m_dViewY0 = 0.0e0;
int QIndicator::m_iViewStep = 10;
int QIndicator::m_dGridStep = 50.0e0;
int QIndicator::m_iMargin = 2;
QColor QIndicator::m_qcGridColor(Qt::darkGray);
int QIndicator::m_iGridThickness = 1;
int QIndicator::m_iAxisThickness = 2;
QColor QIndicator::m_qcPostLabelColor(Qt::darkGreen);
int QIndicator::m_iPostLabelSize=10;
int QIndicator::m_iTickLabelSize=8;
int QIndicator::m_iTickLabelOffsetX=5;
int QIndicator::m_iTickLabelOffsetY=5;
int QIndicator::m_iGridMaxTicks=200;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QIndicator::QIndicator(QIcon &icon, QPoiModel *pPoiModel, QWidget *parent /* =0 */)
        :QGLWidget(parent)
        , m_bIndDragging(false)
        , m_iHighlighted(-1)
        , m_pPoiModel(pPoiModel)
        , m_pFormular (NULL) {
    setWindowIcon(icon);
    setGeometry(m_qrIndGeometry);
	m_dViewX0Pix = m_dViewX0*1.0e3*m_dScale;
	m_dViewY0Pix = m_dViewY0*1.0e3*m_dScale;
    setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this,SIGNAL(customContextMenuRequested(const QPoint &)),SLOT(onCustomContextMenu(const QPoint &)));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QIndicator::~QIndicator() {
    m_qrIndGeometry = geometry();
    if (m_pFormular) {
        delete m_pFormular;
        m_pFormular=NULL;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::closeEvent(QCloseEvent* pe) {
    if (m_pFormular) {
        delete m_pFormular;
        m_pFormular=NULL;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::initializeGL() {
    // Set up the rendering context, define display lists etc.:
    // ...
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    // ...
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::resizeGL(int w, int h) {
    // setup viewport, projection etc.:
    glViewport(0, 0, (GLint)w, (GLint)h);
    //...
    //glFrustum(...);
    //...
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::paintGL() {
    // draw the scene:
    // ...
    //glRotatef(...);
    //glMaterialfv(...);
    glBegin(GL_QUADS);
    //glVertex3f(...);
    //glVertex3f(...);
    //...
    glEnd();
    //...
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::paintEvent(QPaintEvent *qpeEvent) {
	QPainter painter;
	painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
	drawGrid(painter);
	showPosts(painter);
	drawPoints(painter);
	painter.end();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::drawPoints(QPainter &painter) {
	QSize szIndicator = size();
	int iXC=szIndicator.width()/2,iYC=szIndicator.height()/2;
	for (int i=0; i<m_qlPoints.count(); i++) {
		TPoiT *pTPoiT=m_qlPTPoiT.at(i);
        QByteArray baModes((char*)&pTPoiT->m_pRxInfoe->bModeSData[0],sizeof(pTPoiT->m_pRxInfoe->bModeSData));
        if (!m_qsHighlightedModes.isEmpty() && m_qsHighlightedModes!=QString(baModes.toBase64().constData())) continue;
		QPointF pt=m_qlPoints.at(i);
		pt.rx()*=m_dScale;
		pt.ry()*=m_dScale;
		pt.rx()+=m_dViewX0Pix;
		pt.ry()+=m_dViewY0Pix;
		int iType=m_qlPntTypes.at(i);
		if (iType<m_lsLegend.m_iNumColors) {
			int iSize=m_lsLegend.m_qlSizes.at(iType);
            QColor qcColor=m_lsLegend.m_qlColors.at(iType);
            if (i == m_iHighlighted) continue;
			QPen qpPen(qcColor,iSize,Qt::SolidLine,Qt::RoundCap);
		    painter.setPen(qpPen);
		}
		painter.drawPoint(iXC+pt.x(),iYC-pt.y());
	}
    if (m_iHighlighted>=0 && m_iHighlighted<m_qlPoints.count()) {
        QPointF pt=m_qlPoints.at(m_iHighlighted);
        pt.rx()*=m_dScale;
        pt.ry()*=m_dScale;
        pt.rx()+=m_dViewX0Pix;
        pt.ry()+=m_dViewY0Pix;
        QPen qpPen(Qt::red,20,Qt::SolidLine,Qt::RoundCap);
        painter.setPen(qpPen);
        painter.drawPoint(iXC+pt.x(),iYC-pt.y());
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::drawGrid(QPainter &painter) {
	//------------------------------------------------------------------------
	// all coordinates declared here are in widget coordinate system
	// i.e. (0,0) is left upper corner
	//------------------------------------------------------------------------
	double dXC,dYC; // view widget center
	double dX0,dY0; // t.c. coordinate origin (pix) in widget coordinates
    double dX1,dY1, // left upper
           dX2,dY2, // right upper
           dX3,dY3, // right lower
           dX4,dY4;  // left lower
	QSize szIndicator = size();
	dXC=szIndicator.width()/2.0e0;
	dYC=szIndicator.height()/2.0e0;
	m_dViewX0Pix = m_dViewX0*1.0e3*m_dScale;
	m_dViewY0Pix = m_dViewY0*1.0e3*m_dScale;
	dX0=dXC+m_dViewX0Pix;
	dY0=dYC-m_dViewY0Pix;
	dX1=dX4=m_iMargin;
	dX2=dX3=szIndicator.width()-m_iMargin;
	dY1=dY2=m_iMargin;
	dY3=dY4=szIndicator.height()-m_iMargin;

	double dGridStepPix=m_dGridStep*1.0e3*m_dScale;

	int iFromX=qRound((dX1-dX0)/dGridStepPix+0.5e0);
	int iToX  =qRound((dX2-dX0)/dGridStepPix-0.5e0);
	int iFromY=qRound((dY1-dY0)/dGridStepPix+0.5e0);
	int iToY  =qRound((dY3-dY0)/dGridStepPix-0.5e0);

	double dXCur,dYCur;
	QBrush qbGridBrush(m_qcGridColor);
	QPen qpGridPen(qbGridBrush,m_iGridThickness,Qt::SolidLine,Qt::RoundCap);
    painter.setPen(qpGridPen);
	QFont font=QApplication::font();
	font.setWeight(QFont::Normal);
	font.setPointSize(m_iTickLabelSize);
	painter.setFont(font);
	QFontMetrics fmTickLabel(font,this);

	// find the tick label with max width. If it fits not then quit
	int i;
	int iMaxWidth,iSomeWidth;
	QString qsSomeLabel;
	qsSomeLabel=QString::number(iFromX*m_dGridStep,'f',0);
    iMaxWidth=fmTickLabel.width(qsSomeLabel);
	for (i=iFromX; i<=iToX; i++) {
		qsSomeLabel=QString::number(i*m_dGridStep,'f',0);
		iSomeWidth=fmTickLabel.width(qsSomeLabel);
		iMaxWidth=(iSomeWidth>iMaxWidth)?iSomeWidth:iMaxWidth;
	}
	for (i=iFromY; i<=iToY; i++) {
		qsSomeLabel=QString::number(i*m_dGridStep,'f',0);
		iSomeWidth=fmTickLabel.width(qsSomeLabel);
		iMaxWidth=(iSomeWidth>iMaxWidth)?iSomeWidth:iMaxWidth;
	}

	// update ticks range with respect to lable width and height
	int iLabelHeight=fmTickLabel.height();
	iFromX = qRound(( dX1+iMaxWidth+m_iTickLabelOffsetX-dX0)    / dGridStepPix + 0.5e0);
	iToX   = qRound(( dX2-iMaxWidth-m_iTickLabelOffsetX-dX0)    / dGridStepPix - 0.5e0);
	iFromY = qRound((-dY4+iLabelHeight+m_iTickLabelOffsetY+dY0) / dGridStepPix + 0.5e0);
	iToY   = qRound((-dY1-iLabelHeight-m_iTickLabelOffsetY+dY0) / dGridStepPix - 0.5e0);
    if ((iToX-iFromX+1)*iMaxWidth+2*m_iMargin+2*m_iTickLabelOffsetX > szIndicator.width()) return;

	double dGridLeft   =dX0 + iFromX*dGridStepPix;
	double dGridRight  =dX0 + iToX*dGridStepPix;
	double dGridBottom =dY0 - iFromY*dGridStepPix;
	double dGridTop    =dY0 - iToY*dGridStepPix;

	// if inappropriate magnification then skip
	if (   (iToY-iFromY)<1 
		|| (iToX-iFromX)<1
		|| (iToX-iFromX+1)>m_iGridMaxTicks
		|| (iToY-iFromY+1)>m_iGridMaxTicks ) return;

	// actual grid
	for (i=iFromX; i<=iToX; i++) {
		dXCur=dX0+i*dGridStepPix;
		if (dXCur<0 || dXCur>szIndicator.width()) continue;
		QString qsLabel=QString::number(i*m_dGridStep,'f',0);
		int iLabelWidth=fmTickLabel.width(qsLabel);
		painter.drawText(dXCur-iLabelWidth/2,
						 dGridTop-iLabelHeight-m_iTickLabelOffsetY+fmTickLabel.ascent(),
						 qsLabel);
		painter.drawText(dXCur-iLabelWidth/2,
						 dGridBottom+m_iTickLabelOffsetY+fmTickLabel.ascent(),
						 qsLabel);
		if (i==0 || i==iFromX || i==iToX) {
			qpGridPen.setStyle(Qt::SolidLine);
            qpGridPen.setWidth(m_iAxisThickness);
		}
		else {
			qpGridPen.setStyle(Qt::DashLine);
            qpGridPen.setWidth(m_iGridThickness);
		}
        painter.setPen(qpGridPen);
	    painter.drawLine(dXCur,dGridTop,dXCur,dGridBottom);
	}
	for (i=iFromY; i<=iToY; i++) {
		dYCur=dY0-i*dGridStepPix;
		if (dYCur<0 || dYCur>szIndicator.height()) continue;
		QString qsLabel=QString::number(i*m_dGridStep,'f',0);
		int iLabelWidth=fmTickLabel.width(qsLabel);
		painter.drawText(dGridLeft-m_iTickLabelOffsetX-iLabelWidth,
						 dYCur-iLabelHeight/2.0+fmTickLabel.ascent(),
						 qsLabel);
		painter.drawText(dGridRight+m_iTickLabelOffsetX,
						 dYCur-iLabelHeight/2.0+fmTickLabel.ascent(),
						 qsLabel);
		if (i==0 || i==iFromY || i==iToY) {
			qpGridPen.setStyle(Qt::SolidLine);
            qpGridPen.setWidth(m_iAxisThickness);
		}
		else {
			qpGridPen.setStyle(Qt::DashLine);
            qpGridPen.setWidth(m_iGridThickness);
		}
        painter.setPen(qpGridPen);
	    painter.drawLine(dGridLeft,dYCur,dGridRight,dYCur);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::showPosts(QPainter &painter) {
	QSize szIndicator = size();
	double dXC=szIndicator.width()/2.0e0;
	double dYC=szIndicator.height()/2.0e0;
	QPen qpPostLabel(m_qcPostLabelColor);
	painter.setPen(qpPostLabel);
	QFont font=QApplication::font();
	font.setWeight(QFont::Bold);
	font.setPointSize(m_iPostLabelSize);
	painter.setFont(font);
	QFontMetrics fmPostLabel(font,this);
	for (int i=0; i<m_qlPostsId.count(); i++) {
		if (i>=m_qlPostsCoord.count()) return;
		int iId=m_qlPostsId.at(i);
		QString qsLabel=QString::number(iId);
		int iLabelWidth=fmPostLabel.width(qsLabel);
		int iLabelHeight=fmPostLabel.height();
		QPointF qpPost=m_qlPostsCoord.at(i);
		double dPostX=m_dViewX0Pix+qpPost.x()*m_dScale;
		double dPostY=m_dViewY0Pix+qpPost.y()*m_dScale;
		painter.drawText(dXC+dPostX-iLabelWidth/2.0e0,
			             dYC-dPostY-iLabelHeight/2.0e0+fmPostLabel.ascent(),
						 qsLabel);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void QIndicator::keyReleaseEvent(QKeyEvent *pe) {
	if (pe->key()==Qt::Key_Up
	 || pe->key()==Qt::Key_Right 
	 || pe->key()==Qt::Key_Down 
	 || pe->key()==Qt::Key_Left) {
        switch (pe->key()) {
			case Qt::Key_Up:
                m_dViewY0Pix += m_iViewStep; break;
			case Qt::Key_Down:
                m_dViewY0Pix -= m_iViewStep; break;
			case Qt::Key_Right:
                m_dViewX0Pix += m_iViewStep; break;
			case Qt::Key_Left:
                m_dViewX0Pix -= m_iViewStep; break;
		}
		m_dViewX0 = m_dViewX0Pix / m_dScale * 1.0e-3;
		m_dViewY0 = m_dViewY0Pix / m_dScale * 1.0e-3;
        repaint();
		return;
	}
	else if ((pe->key()==Qt::Key_Equal
	      ||  pe->key()==Qt::Key_Minus)  
	      && (pe->modifiers() & Qt::ControlModifier)) {
        switch (pe->key()) {
			case Qt::Key_Equal:
                m_dScale  = qBound(m_dScaleMin,m_dScale*1.1e0,m_dScaleMax);
				break;
			case Qt::Key_Minus:
                m_dScale  = qBound(m_dScaleMin,m_dScale/1.1e0,m_dScaleMax);
				break;
		}
		m_dViewX0Pix = m_dViewX0*1.0e3*m_dScale;
		m_dViewY0Pix = m_dViewY0*1.0e3*m_dScale;
        repaint();
		return;
	}
	else if ( pe->key()==Qt::Key_0 && pe->modifiers() & Qt::ControlModifier) {
		m_dViewX0Pix = m_dViewX0 = 0.0e0;
		m_dViewY0Pix = m_dViewY0 = 0.0e0;
		m_dScale = m_dDefaultScale;
        repaint();
		return;
	}
    // allow all other events
	QGLWidget::keyPressEvent(pe);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::indicatorUpdate() {
    updateGL();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::addPoint(double x,double y, int iType, TPoiT *pTPoiT, int iRawIdx) {
	m_qlPoints.append(QPoint(x,y));
	m_qlPntTypes.append(iType);
	m_qlPTPoiT.append(pTPoiT);
    m_qlIdxPoite.append(iRawIdx);
    // updateGL();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::addPost(double x, double y, int iId) {
	if (m_qlPostsId.contains(iId)) {
		int iCurr=m_qlPostsId.indexOf(iId);
		m_qlPostsCoord.replace(iCurr,QPointF(x,y));
		return;
	}
	m_qlPostsId.append(iId);
	m_qlPostsCoord.append(QPointF(x,y));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void QIndicator::mousePressEvent(QMouseEvent *pe) {
    if (pe->button() == Qt::LeftButton) {
        m_qpLastPoint = pe->pos();
        m_bIndDragging = true;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void QIndicator::mouseMoveEvent(QMouseEvent *pe) {
	if ((pe->buttons() & Qt::LeftButton) && m_bIndDragging) {
        indicatorDrag(pe->pos());
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::onFormularAccepted() {
    if (m_pFormular) {
        m_qsHighlightedModes=m_pFormular->m_qsModeSData;
        delete m_pFormular;
        m_pFormular=NULL;
    }
    else {
        m_qsHighlightedModes=QString();
    }
    m_iHighlighted=-1;
    repaint();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::onFormularRejected() {
    if (m_pFormular) {
        delete m_pFormular;
        m_pFormular=NULL;
    }
    m_qsHighlightedModes=QString();
    m_iHighlighted=-1;
    repaint();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void QIndicator::mouseReleaseEvent(QMouseEvent *pe) {
    if (pe->button() == Qt::LeftButton && m_bIndDragging) {
        indicatorDrag(pe->pos());
        m_bIndDragging = false;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::indicatorDrag(const QPoint &qpEndPoit) {
    int iDeltaX =  qpEndPoit.x() - m_qpLastPoint.x();
    int iDeltaY =-(qpEndPoit.y() - m_qpLastPoint.y());

	m_dViewX0Pix += iDeltaX;
	m_dViewY0Pix += iDeltaY;

	m_dViewX0 = m_dViewX0Pix / m_dScale * 1.0e-3;
	m_dViewY0 = m_dViewY0Pix / m_dScale * 1.0e-3;

    m_qpLastPoint = qpEndPoit;
	repaint();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* virtual */ void QIndicator::wheelEvent(QWheelEvent *pe) {
	if (pe->modifiers() & Qt::ControlModifier) {
		QSize szIndicator = size();
		int iXC=szIndicator.width()/2;
		int iYC=szIndicator.height()/2;
		// event coordinates (pix) relative to widget center
		double dEventX=  pe->x()-iXC ; 
		double dEventY=-(pe->y()-iYC);
		// magnification coefficient
		double dMagnCoef=1.0e0;
		int iNumSteps=abs(pe->delta());
		bool bMagnify=(pe->delta()>0);
		for (int i=0; i<iNumSteps; i++) {
			dMagnCoef=bMagnify?(dMagnCoef*1.001e0):(dMagnCoef/1.001e0);
		}
		// new location of topocentric origin (pix) relative to widget center
		m_dViewX0Pix=m_dViewX0Pix*dMagnCoef - dEventX*(dMagnCoef-1.0e0);
		m_dViewY0Pix=m_dViewY0Pix*dMagnCoef - dEventY*(dMagnCoef-1.0e0);
		// scale update
		m_dScale  = qBound(m_dScaleMin,dMagnCoef*m_dScale,m_dScaleMax);
		// new location of topocentric origin (km)
		m_dViewX0 = m_dViewX0Pix*1.0e-3/m_dScale;
		m_dViewY0 = m_dViewX0Pix*1.0e-3/m_dScale;
		repaint();
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicator::onCustomContextMenu(const QPoint &qpPos) {

    if (QVoiProcessor::m_bUseGen) return;

    int iXC,iYC; // view widget center
    QSize szIndicator = size();
    iXC=szIndicator.width()/2.0e0;
    iYC=szIndicator.height()/2.0e0;
    int iLegendTypePrimary = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_PRIMARY);

    QList<double> qlRnn;
    QList<int> qlInn;
    for (int i=0; i<m_qlPoints.count(); i++) {
        QPointF pt=m_qlPoints.at(i);
        double dTgX=pt.rx();
        double dTgY=pt.ry();
        pt.rx()*=m_dScale;
        pt.ry()*=m_dScale;
        pt.rx()+=m_dViewX0Pix;
        pt.ry()+=m_dViewY0Pix;
        int iType=m_qlPntTypes.at(i);
        if (!m_qsHighlightedModes.isEmpty()) {
            TPoiT *pTPoiT=m_qlPTPoiT.at(i);
            QByteArray baModes((char*)&pTPoiT->m_pRxInfoe->bModeSData[0],sizeof(pTPoiT->m_pRxInfoe->bModeSData));
            QString qsModes=QString(baModes.toBase64().constData());
            if (m_qsHighlightedModes!=qsModes) continue;
        }
        if (iType == iLegendTypePrimary) {
            double xx=iXC+pt.x()-qpPos.x();
            double yy=iYC-pt.y()-qpPos.y();
            qlRnn.append(abs(xx)+abs(yy));
            qlInn.append(i);
        }
    }
    if (qlRnn.count()==0) {
        m_qsHighlightedModes = QString();
        m_iHighlighted=-1;
        repaint();
        return;
    }
    if (qlRnn.count()>1) {
        for (int i=0; i<qlRnn.count()-1; i++) {
            for (int j=i+1; j<qlRnn.count(); j++) {
                if (qlRnn.at(i)>qlRnn.at(j)) {
                    qlRnn.swap(i,j);
                    qlInn.swap(i,j);
                }
            }
        }
    }
    m_iHighlighted=qlInn.at(0);
    repaint();
    QPointF pt=m_qlPoints.at(m_iHighlighted);
    TPoiT *pTPoiT=m_qlPTPoiT.at(m_iHighlighted);
    int iIdxPoite=m_qlIdxPoite.at(m_iHighlighted);
    QByteArray baPoite;
    if (iIdxPoite >= 0 && iIdxPoite<m_pPoiModel->getRawListSize()) {
        baPoite=m_pPoiModel->getPPoite(iIdxPoite);
    }
    else {
        throw RmoException(QString("iIdxPoite(%1) >= 0 && iIdxPoite<m_pPoiModel->getRawListSize() (%2)")
                           .arg(iIdxPoite).arg(m_pPoiModel->getRawListSize()));
    }
    if (m_pFormular) {
        delete m_pFormular;
        m_pFormular = NULL;
    }
    qint64 iTime=m_pPoiModel->m_qlPoiteTime.at(iIdxPoite);
    // show formular only if baPoite is valid POITE
    if (!baPoite.isEmpty()) {
        m_pFormular = new QFormular(iTime,pt,pTPoiT,baPoite,this);
        QObject::connect(m_pFormular,SIGNAL(accepted()),this,SLOT(onFormularAccepted()));
        QObject::connect(m_pFormular,SIGNAL(rejected()),this,SLOT(onFormularRejected()));
        m_pFormular->show();
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QFormular::QFormular(qint64 iTime, QPointF pfLoc, TPoiT *pTPoiT, QByteArray baPoite,
                     QIndicator *pOwner,QWidget *parent /*= 0*/)
        : QDialog(parent,Qt::MSWindowsFixedSizeDialogHint)
        , m_baPoite(baPoite)
        , m_pOwner(pOwner)
        , m_qsSmodeCall(QString())
        , m_qsSmodeAdr(QString())
        , m_qsModeSData(QString())
        {
    // setup UI
    setWindowIcon(QIcon(QPixmap(":/Resources/formular.ico")));
    QPushButton *ppbOk = new QPushButton(tr("Ok"));
    QPushButton *ppbCancel = new QPushButton(tr("Cancel"));
    QVBoxLayout *pLayout=new QVBoxLayout;

    // m_qsModeSData
    QByteArray baModes((char*)&pTPoiT->m_pRxInfoe->bModeSData[0],sizeof(pTPoiT->m_pRxInfoe->bModeSData));
    m_qsModeSData=QString(baModes.toBase64().constData());

    // formular table widget
    m_pTable=new QTableWidget(this);
    m_pTable->setColumnCount(2);
    m_pTable->horizontalHeader()->hide();
    m_pTable->verticalHeader()->hide();
    m_pTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    int iRowNum=0;
    m_pTable->insertRow(iRowNum);
    m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("Time stamp")));
    m_pTable->setItem(iRowNum,1,new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(iTime).toString("hh:mm:ss.zzz")));
    iRowNum++;
    m_pTable->insertRow(iRowNum);
    m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("Coord km")));
    m_pTable->setItem(iRowNum,1,new QTableWidgetItem(QString("(%1, %2)").arg(pfLoc.rx()*1.0e-3,0,'f',1).arg(pfLoc.ry()*1.0e-3,0,'f',1)));
    iRowNum++;
    m_pTable->insertRow(iRowNum);
    m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("Freq MHz")));
    m_pTable->setItem(iRowNum,1,new QTableWidgetItem(QString("%1").arg(pTPoiT->m_pRxInfoe->dF)));
    iRowNum++;
    m_pTable->insertRow(iRowNum);
    m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("Ampl dB")));
    m_pTable->setItem(iRowNum,1,new QTableWidgetItem(QString("%1").arg(pTPoiT->m_pRxInfoe->dAmp)));
    iRowNum++;
    m_pTable->insertRow(iRowNum);
    m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("bModeSData[]")));
    m_pTable->setItem(iRowNum,1,new QTableWidgetItem(m_qsModeSData.mid(0,12)+"..."));
    // add detailed info from POITE structure
    // +   unsigned short uMinuIndx, uSubtIndx;  // Гео координаты постов (ссылки к blh)
    // -   int iIndex;             // номер имп. в пачке от 0
    // -   unsigned long uT;       // uT - Временное смещение от начала СЛ в нс
    //    union {
    //    double Az;				// Азимут а радианах
    // +  double D;				// рх в м
    //    };
    //    union {
    //    double sAz;				// СКО азимута в рад
    // -  double sD;				// СКО рх в м
    //    };
    // +    double dAmp;			// Амплитуда дБ
    // +    double dTau;			// Длит имп мкс
    // +    double dF;				// Цент. частота Мгц
    // -    DWORD dwFrSigma2;
    // -    DWORD dwStrobEQuad;
    // +    DWORD dwPeriod;			// период в пачке в нс
    // +   USHORT usImpCount;		// число имп. в пачке
    // +   short sFreq0, sFreq1;	// KHz от центральной
    //    USHORT usStrobE;
    //    union {
    //    struct {
    //        DWORD D4_C1_UBD;
    //        DWORD DP1;
    //        DWORD DP2;
    //        DWORD DP3; };
    // +   BYTE bModeSData[16]; };
    // +    USHORT usSifMode;
    //     USHORT usFlags;
    // -   int iNTacan;

    if (!baPoite.isEmpty()) {
        PPOITE pPoite = (PPOITE) m_baPoite.data();
        iRowNum++;
        m_pTable->insertRow(iRowNum);
        m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("flags")));
        m_pTable->setItem(iRowNum,1,new QTableWidgetItem(QString("%1").arg(pPoite->uFlags,0,16)));
        iRowNum++;
        if (pPoite->iSmodeAdr!=-1) m_qsSmodeAdr=QString("0x%1").arg(pPoite->iSmodeAdr,0,16);
        m_pTable->insertRow(iRowNum);
        m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("S-Mode address")));
        m_pTable->setItem(iRowNum,1,new QTableWidgetItem(m_qsSmodeAdr));
        if (pPoite->bSmodeCall[0]!='\0') {
            QByteArray ba(&pPoite->bSmodeCall[0], 8);
            m_qsSmodeCall = QString(ba);
        }
        iRowNum++;
        m_pTable->insertRow(iRowNum);
        m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("S-Mode call")));
        m_pTable->setItem(iRowNum,1,new QTableWidgetItem(m_qsSmodeCall));
        iRowNum++;
        m_pTable->insertRow(iRowNum);
        m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("Bases")));
        m_pTable->setItem(iRowNum,1,new QTableWidgetItem(pTPoiT->getBasesString()));
        if (pPoite->Count == pTPoiT->m_iMatchCount) {
            RXINFOE *prx;
            // path diffs
            QString qsPathDiffs("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                qsPathDiffs.append(QString(" %1,").arg(prx->D,0,'f',0));
                prx++;
            }
            qsPathDiffs.chop(1);
            qsPathDiffs.append("]");
            iRowNum++;
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("PathDiffs")));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsPathDiffs));
            // calc path diffs
            iRowNum++;
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("CalcPathDiffs")));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(pTPoiT->calculatedPathDiffs(pfLoc.rx(),pfLoc.ry())));
            // freq
            QString qsFreq("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                double dFreq=qBound(-1.,prx->dF,100000.);
                qsFreq.append(QString(" %1,").arg(dFreq,0,'f',1));
                prx++;
            }
            qsFreq.chop(1); qsFreq.append("]");
            iRowNum++;
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(tr("Freq MHz")));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsFreq));
            // all kinds of data
            QString qsData, qsDataTitle;
            int iRowNum;
            // Az
            iRowNum=8; qsDataTitle=QString("Azimuth"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                double dData=qBound(-100000.,prx->Az,100000.);
                qsData.append(QString(" %1,").arg(dData,0,'f',1));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            iRowNum++;
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // Tau
            iRowNum++; qsDataTitle=QString("Tau"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                double dData=qBound(-100000.,prx->dTau,100000.);
                qsData.append(QString(" %1,").arg(dData,0,'f',1));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // usSifMode
            iRowNum++; qsDataTitle=QString("SIF mode"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                qsData.append(QString(" %1,").arg(prx->usSifMode));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // ModeS data
            iRowNum++; qsDataTitle=QString("ModeS data"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                QByteArray baModesData((char*)&prx->bModeSData[0],sizeof(prx->bModeSData));
                QString qsModesData(baModesData.toBase64().constData());
                qsData.append(QString(" %1..,").arg(qsModesData.mid(0,4)));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // imp index
            iRowNum++; qsDataTitle=QString("rx.iIndex"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                qsData.append(QString(" %1,").arg(prx->iIndex));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // Amp
            iRowNum++; qsDataTitle=QString("Ampl dB"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                double dData=qBound(-100000.,prx->dAmp,100000.);
                qsData.append(QString(" %1,").arg(dData,0,'f',1));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // usImpCount
            iRowNum++; qsDataTitle=QString("ImpCount"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                qsData.append(QString(" %1,").arg(prx->usImpCount));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // dwPeriod
            iRowNum++; qsDataTitle=QString("Period"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                double dData=qBound(-100000.,1.*prx->dwPeriod,100000.);
                qsData.append(QString(" %1,").arg(dData,0,'f',1));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // sFreq0
            iRowNum++; qsDataTitle=QString("F0 kHz"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                qsData.append(QString(" %1,").arg(prx->sFreq0));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
            // sFreq1
            iRowNum++; qsDataTitle=QString("F1 kHz"); qsData=QString("[");
            prx= &pPoite->rx[0];
            for (int i=0; i<pPoite->Count; i++) {
                qsData.append(QString(" %1,").arg(prx->sFreq1));
                prx++;
            }
            qsData.chop(1); qsData.append("]");
            m_pTable->insertRow(iRowNum);
            m_pTable->setItem(iRowNum,0,new QTableWidgetItem(qsDataTitle));
            m_pTable->setItem(iRowNum,1,new QTableWidgetItem(qsData));
        }
    }
    m_pTable->resizeColumnsToContents();
    m_pTable->resizeRowsToContents();
    pLayout->addWidget(m_pTable);
    QHBoxLayout *pHLayout=new QHBoxLayout;
    pHLayout->addStretch();
    pHLayout->addWidget(ppbOk);
    pHLayout->addWidget(ppbCancel);
    pLayout->addLayout(pHLayout);
    setLayout(pLayout);

    QObject::connect(ppbOk,SIGNAL(pressed()),this,SLOT(accept()));
    QObject::connect(ppbCancel,SIGNAL(pressed()),this,SLOT(reject()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QFormular::~QFormular() {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFormular::showEvent(QShowEvent *event) {
    QRect rec = QApplication::desktop()->screenGeometry();
    int iScrWid = rec.width();
    int iTotWidth=0;
    for (int i=0; i<m_pTable->columnCount(); i++) {
        iTotWidth+=m_pTable->columnWidth(i);
    }
    int iTotHeight=0;
    for (int i=0; i<m_pTable->rowCount(); i++) {
        iTotHeight+=m_pTable->rowHeight(i);
    }
    iTotWidth+=m_pTable->lineWidth()*(m_pTable->columnCount()+1);
    iTotWidth+=width()-m_pTable->width();
    iTotHeight+=m_pTable->lineWidth()*(m_pTable->rowCount()+1);
    iTotHeight+=height()-m_pTable->height();
    resize(iTotWidth,iTotHeight);
    move(iScrWid-frameSize().width()-3,3);
}
