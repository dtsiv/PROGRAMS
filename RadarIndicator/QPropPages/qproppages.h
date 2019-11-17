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

	// public-visible interface controls
public:
    QLineEdit *m_pleDBFileName;
    QPushButton *m_pbAccept;
    QLineEdit *m_pleFCarrier;
    QLineEdit *m_pleTSampl;
    QCheckBox *m_pcbAdaptiveGrid;
    QCheckBox *m_pcbBeamsUsedForPeleng[QPROPPAGES_NBEAMS*(QPROPPAGES_NBEAMS-1)/2];

private:
    QObject *m_pOwner;
    QTabWidget *m_pTabWidget;
};


#endif // QPROPPAGES_H
