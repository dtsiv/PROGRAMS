#ifndef QPROPPAGES_H
#define QPROPPAGES_H

#include <QtGui>
#include <QtWidgets>
#include <QTabWidget>
#include <QComboBox>

#define QPROPPAGES_ACTIVE_TAB                        "ActivePropertyPage"
#define QPROPPAGES_NBEAMS                            4
#define PROP_LINE_WIDTH                              60

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
class QPropPages : public QDialog {
	Q_OBJECT

public:
    QPropPages(QObject *pOwner, QWidget *parent = 0);
	~QPropPages();

public slots:
    void onAccepted();

signals:
    void doParse();
    void chooseRegFile();
    void updateParseProgressBar(double dCurr);
    void updateGenerateNoiseMapProgressBar(double dCurr);
    void chooseSqliteFile();
    void chooseNoiseMapFile();
    void generateNoiseMapFile();

	// public-visible interface controls
public:
    QLineEdit *m_pleDBFileName;
    QLineEdit *m_pleRegFileName;
    QPushButton *m_ppbAccept;
    QPushButton *m_ppbParse;
    QLineEdit *m_pleFCarrier;
    QLineEdit *m_pleTSampl;
    QCheckBox *m_pcbParseWholeDir;
    QCheckBox *m_pcbAdaptiveGrid;
    QCheckBox *m_pcbBeamsUsedForPeleng[QPROPPAGES_NBEAMS*(QPROPPAGES_NBEAMS-1)/2];
    QProgressBar *m_ppbarParseProgress;
    QLineEdit *m_pleTimerMSecs;
    QLineEdit *m_pleNoiseMapFName;
    QCheckBox *m_pcbUseNoiseMap;
    QProgressBar *m_ppbarGenerateNoiseMap;
    QPushButton *m_ppbGenerateNoiseMap;
    QLineEdit *m_pleTargSzThresh;
    QLineEdit *m_pleAntennaSzAz;
    QLineEdit *m_pleAntennaSzEl;
    QLineEdit *m_pleBeamOffsetD0;
    QComboBox *m_pcbbWeighting;
    QComboBox *m_pcbbRejector;
    QLineEdit *m_pleFalseAlarmP;
    QLineEdit *m_pleMaxTgSize;

private:
    QObject *m_pOwner;
    QTabWidget *m_pTabWidget;
};


#endif // QPROPPAGES_H
