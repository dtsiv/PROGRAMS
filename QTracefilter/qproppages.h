#ifndef QPROPPAGES_H
#define QPROPPAGES_H

#include <QtGui>
#include <QSettings>
#include <QtWidgets>
#include "ui_qproppages.h"
#include "qgeoutils.h"
#include "qpoimodel.h"

#define PROP_LINE_WIDTH                         60

//========= proppage captions =============
#define PROPPAGE_DB                             "DB connection"
#define PROPPAGE_MAINCTRL                       "Config"
#define PROPPAGE_KALMAN                         "Filter"
#define PROPPAGE_IMITATOR                       "Imitator"
#define PROPPAGE_INDICATOR                      "Indicator"

//========= settings keys =============
#define SETTINGS_KEY_PGCONN                     "PostgresConnectionString"
#define SETTINGS_SQLITE_FILE                    "SQLiteFile"
#define SETTINGS_KEY_MAINCTRL                   "MainCtrl"
#define SETTINGS_KEY_DBENGINE                   "DBEngine"
#define SETTINGS_KEY_FLT_SIGMAS                 "KalmanSigmaS"
#define SETTINGS_KEY_FLT_SIGMAM                 "KalmanSigmaM"
#define SETTINGS_KEY_FLT_HEIGHT                 "KalmanHeight"
#define SETTINGS_KEY_FLT_USE_TOL                "KalmanUseTolerance"
#define SETTINGS_KEY_FLT_MINSTROB               "KalmanMinStrob"
#define SETTINGS_KEY_FLT_STROBSPREAD            "KalmanStrobSpread"
#define SETTINGS_KEY_FLT_CHI2PROB               "KalmanChi2Prob"
#define SETTINGS_KEY_FLT_MATRELAX               "KalmanMatRelax"
#define	SETTINGS_KEY_FLT_USERELAX               "KalmanUseRelax"
#define SETTINGS_KEY_USE_GEN                    "UseImitator"
#define SETTINGS_KEY_TRAJDUR                    "TrajectoryDuration"
#define SETTINGS_KEY_FLT_TOLERANCE              "KalmanTolerance"
#define SETTINGS_KEY_GEN_SIGMAM                 "GeneratorSigmaM"
#define SETTINGS_KEY_GEN_DELAY                  "GeneratorDelay"
#define SETTINGS_KEY_GEN_X0                     "GeneratorX0"
#define SETTINGS_KEY_GEN_Y0                     "GeneratorY0"
#define SETTINGS_KEY_GEN_V0                     "GeneratorV0"
#define SETTINGS_KEY_GEN_AZ0                    "GeneratorAz0"
#define SETTINGS_KEY_GEN_TRAJ_R                 "GeneratorTrajR"
#define SETTINGS_KEY_GEN_TRAJ_CW                "GeneratorTrajCW"
#define SETTINGS_KEY_GEN_HEIGHT                 "GeneratorHeight"
#define SETTINGS_KEY_GEN_KINKTIME               "GeneratorKinkTime"
#define SETTINGS_KEY_GEN_KINKANGLE              "GeneratorKinkAngle"
#define SETTINGS_KEY_GEN_TRAJTYPE               "GeneratorTrajType"
#define SETTINGS_KEY_PROP_TAB                   "PropTab"
#define SETTINGS_KEY_GEOMETRY                   "geometry"
#define SETTINGS_KEY_IND_GEOMETRY               "IndGeometry"
#define SETTINGS_KEY_IND_SCALE                  "IndViewScale"
#define SETTINGS_KEY_IND_VIEWX0                 "IndViewX0"
#define SETTINGS_KEY_IND_VIEWY0                 "IndViewY0"
#define SETTINGS_KEY_IND_GRIDSTEP               "IndGridStep"
#define SETTINGS_KEY_TIMESLICE                  "TimeSlice"
#define SETTINGS_KEY_TICKIDX                    "TickIndex"
#define SETTINGS_KEY_SYMBSIZES                  "SymbolSizes"

// color selection button
class QColorSelectionButton : public QPushButton {
	Q_OBJECT

public:
    QColorSelectionButton(const QColor &c, QWidget *parent=0);
	~QColorSelectionButton();
	QColor& getSelection();

private slots:
	void onClicked();
    void onColorSelected(const QColor &color);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
	QColor m_qcColorSelection;
	QColorDialog *m_pcdColorDlg;
};

// dialog with property tabs
class QTraceFilter;

class QPropPages : public QDialog {
	Q_OBJECT

public:
	QPropPages(QTraceFilter *pOwner,QMap<QString,QVariant> &qmParamDefaults,QWidget *parent = 0);
	~QPropPages();

	QGraphicsScene m_scene;

public slots:
	void onMainctrlChanged(QString qsNewText);
	void onMainctrlChoose();
	void onSQLiteFileChoose();

public:
	QLineEdit *m_pleDbConn;
	QLineEdit *m_pleSqliteDbFile;
	QLineEdit *m_pleMainCtrl;

	QLineEdit *m_pleKalmanSigmaS;
	QLineEdit *m_pleKalmanSigmaM;
	QLineEdit *m_pleKalmanHeight;
	QLineEdit *m_pleKalmanTolerance;
	QCheckBox *m_pcbKalmanUseTol;
	QLineEdit *m_pleKalmanMinStrob;
	QLineEdit *m_pleKalmanStrobSpread;
	QComboBox *m_pcbKalmanChi2Prob;
	QCheckBox *m_pcbKalmanUseRelax;
	QLineEdit *m_pleKalmanMatRelaxTime;

	QLineEdit *m_pleGenX0;
	QLineEdit *m_pleGenY0;
	QLineEdit *m_pleGenInitAz;
	QLineEdit *m_pleGenInitV;
	QLineEdit *m_pleGenTrajR;
	QLineEdit *m_pleGenSigmaM;
	QLineEdit *m_pleGenDelay;
	QLineEdit *m_pleGenHeight;
	QLineEdit *m_pleGenKinkTime;
	QLineEdit *m_pleGenKinkAngle;
	QLineEdit *m_pleTrajDuration;
	QMap<QString,QColorSelectionButton*> m_qmIndLegendColors;
	QMap<QString,QSpinBox*> m_qmIndSymbSizes;

	QCheckBox *m_pcbGenTrajRotCW;

	QLineEdit *m_pleIndViewScale;
	QLineEdit *m_pleIndViewX0;
	QLineEdit *m_pleIndViewY0;
	QLineEdit *m_pleIndGridStep;

	QPoiModel::POI_DB_ENGINES getSelectedDBEngine();

private:
	void addTab(int iIdx);
	void showPosts(QString qsMainctrlCfg);

	Ui::QPropPages ui;

	QTraceFilter *m_pOwner;
	QStringList m_qslPages;
	QLabel *m_plbAvrBL;
	BLH m_blhViewPoint;
	QRadioButton *m_prbDBEngineSqlite;
	QRadioButton *m_prbDBEnginePostgres;
	QMap<QString,QVariant> m_qmParamDefaults;
	QButtonGroup *m_pbgDBEngine;

};

#endif // QPROPPAGES_H
