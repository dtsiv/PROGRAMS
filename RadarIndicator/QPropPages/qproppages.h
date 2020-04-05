#ifndef QPROPPAGES_H
#define QPROPPAGES_H

#include <QtGui>
#include <QtWidgets>
#include <QTabWidget>

#define QPROPPAGES_ACTIVE_TAB                        "ActivePropertyPage"
#define QPROPPAGES_NBEAMS                            4

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
    void updateProgressBar(double dCurr);
    void chooseSqliteFile();

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

private:
    QObject *m_pOwner;
    QTabWidget *m_pTabWidget;
};


#endif // QPROPPAGES_H
