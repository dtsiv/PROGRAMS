#ifndef QINDICATOR_H
#define QINDICATOR_H

#include <QtGui>
#include <QtWidgets>
#include <QGLWidget>
#include "tpoit.h"

#define DEFAULT_SYMBOL_SIZE                5

#define QINDICATOR_LEGEND_PRIMARY          "Primary points"
#define QINDICATOR_LEGEND_SOURCE           "Imitator source"
#define QINDICATOR_LEGEND_FILTER           "Filter output"
#define QINDICATOR_LEGEND_SOURCE_ALARM     "Source alarm"
#define QINDICATOR_LEGEND_CLUSTER          "Primary cluster"

struct LegendSettings {
	int m_iNumColors;
    QStringList m_qlSettingNames;
    QList<QColor> m_qlColors;
	QList<int> m_qlSizes;
};

class QFormular;
class QPoiModel;

class QIndicator : public QGLWidget {
	Q_OBJECT

public:
    QIndicator(QIcon &icon, QPoiModel *pPoiModel, QWidget *parent = 0);
	~QIndicator();

    static LegendSettings m_lsLegend;
	static QRect m_qrIndGeometry;
	static double m_dScale; // pixels per meter
	static double m_dDefaultScale; // pixels per meter
	static double m_dScaleMax; // pixels per meter
	static double m_dScaleMin; // pixels per meter
	static double m_dViewX0; // View center coordinates km
	static double m_dViewY0; // View center coordinates km
	static int m_iViewStep;
	static int m_dGridStep; // grid step km
	static int m_iMargin;
	static QColor m_qcGridColor;
	static int m_iGridThickness;
	static int m_iAxisThickness;
	static QColor m_qcPostLabelColor;
	static int m_iPostLabelSize;
	static int m_iTickLabelSize;
	static int m_iTickLabelOffsetX;
	static int m_iTickLabelOffsetY;
	static int m_iGridMaxTicks; // gen kink angle deg

public slots:
	void addPost(double x, double y, int iId);
    void addPoint(double x, double y, int iType, TPoiT *pTPoit, int iIdxPoite);
    void indicatorUpdate();
    void onCustomContextMenu(const QPoint &);
    void onFormularAccepted();
    void onFormularRejected();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
	void paintEvent(QPaintEvent *qpeEvent);
	virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void closeEvent(QCloseEvent* pe);

private:
	void indicatorDrag(const QPoint &qpEndPoit);
	void drawGrid(QPainter &painter);
    void drawPoints(QPainter &painter);
    void showPosts(QPainter &painter);

	QList<TPoiT*> m_qlPTPoiT;
    QList<int> m_qlIdxPoite;
    QList<QPointF> m_qlPoints;
	QList<QPointF> m_qlPostsCoord;
	QList<int> m_qlPostsId;
	QList<int> m_qlPntTypes;
	double m_dViewX0Pix; // topocentric origin (X0,Y0) location (pix) relative to widget center 
	double m_dViewY0Pix; // topocentric origin (X0,Y0) location (pix) relative to widget center 
    QPoint m_qpLastPoint;
	bool m_bIndDragging;
	int m_iHighlighted;
    QString m_qsHighlightedModes;
    QFormular *m_pFormular;
    QPoiModel *m_pPoiModel;
    friend class QFormular;
};

class QFormular : public QDialog {
    Q_OBJECT

public:
    QFormular(qint64 iTime, QPointF pfLoc, TPoiT *pTPoiT, QByteArray baPoite, QIndicator *pOwner,QWidget *parent = 0);
    ~QFormular();
    void showEvent(QShowEvent *event);
    QString m_qsSmodeCall;
    QString m_qsSmodeAdr;
    QString m_qsModeSData;

private:
    QIndicator *m_pOwner;
    QTableWidget *m_pTable;
    QByteArray m_baPoite;
};

#endif // QINDICATOR_H
