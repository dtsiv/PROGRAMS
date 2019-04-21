#ifndef QTraceFilter_H
#define QTraceFilter_H

#include <QMainWindow>
#include <QSettings>
#include "ui_QTraceFilter.h"
#include "qgeoutils.h"
#include "qserial.h"
#include "qproppages.h"
#include "qpoimodel.h"
#include "qvoiprocessor.h"
#include "tpoit.h"
#include "qindicator.h"
#include "rmoexception.h"
#include "qexceptiondialog.h"

#define CONN_STATUS_POSTGRES "Connected to PostreSQL"
#define CONN_STATUS_SQLITE   "Connected to SQLite"
#define CONN_STATUS_DISCONN  "Disconnected from DB"

class QTraceFilter : public QMainWindow
{
	Q_OBJECT

public:
    QTraceFilter(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~QTraceFilter();

    void readSettings();
	void writeSettings();
	void addPoint(int x,int y);
	void closeView();

	static quint32 m_uTimeSlice;
	static quint32 m_uTickIdx;

public slots:
	void onValueChanged(int iVal);
	void onUpdate();
	void onClosed();
	void onSetup();
	void onSliceSelected(int iIndex);
	void onControlToggled();
	void onPostSkipped(int id);
	void onStartup();
	bool connectToDatabase();
	void displayConnStatus(QPoiModel::POI_DB_ENGINES eCurrEngine);

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void defaultParameterValues();
	void arrangeControls();
	void refreshTimeSlider();

	Ui::QTraceFilterClass ui;

	QSettings *m_pSettings;
	QPoiModel *m_pPoiModel;
	QAction *exitAct;
    QAction *aboutAct;
	QAction *updateAct;
	QAction *settingsAct;
	QMenu *fileMenu;
	QMenu *helpMenu;
	QLabel *lbStatusMsg;
	QLabel *lbStatusArea;
    QComboBox *comboTimeSlice;
    QSlider *horizontalSlider;
	QLabel *lblTime;
	QPushButton *buttonClose;
	QPushButton *buttonUpdate;
	QRadioButton *prbPoiteDB;
	QRadioButton *prbGen;
	QComboBox *m_pcbTraj;
	QGroupBox *m_pgbImit;
	QGroupBox *m_pgbPoiteDb;

	QIndicator *m_pIndicator;

	QList<int> m_qlTimeSlices;
	quint32 m_uMaxTick;
	quint64 m_tFrom, m_tTo;
	quint64 m_uMsecsTot;
	quint64 m_currTimeMSecs;
	quint64 m_tSliceFrom,m_tSliceTo;

	QString m_qsMainctrlCfg;
	QString m_qsPgConnStr;
	QString m_qsSqliteFilePath;
	QPoiModel::POI_DB_ENGINES m_dbeEngineSelected;

	int m_iActiveTab;

	const int MAX_TICK;
	const QString TIME_LBL_FMT;

	QMap<QString,QVariant> m_qmParamDefaults;
	QButtonGroup *m_pbgTrajectoryType;
	QButtonGroup *m_pbgSkipPost;

public:
	bool m_bSetup;

	friend class QPropPages;
};

//*****************************************************************************
//
//*****************************************************************************
class UserControlInputFilter : public QObject {
protected:
	virtual bool eventFilter(QObject*, QEvent*);

public:
    UserControlInputFilter(QTraceFilter* pOwner, QObject *pobj=0);

private:
	QTraceFilter *m_pOwner;
};

#endif // QTraceFilter_H

//*****************************************************************************
//
//*****************************************************************************
class QStopper : public QSplashScreen {
public:
	QStopper();
	~QStopper();

protected:
	virtual void drawContents (QPainter *painter);

private:
	QPixmap m_pmHarddisk;
};

