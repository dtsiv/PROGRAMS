#ifndef QPROPPAGES_H
#define QPROPPAGES_H

#include <QtGui>
#include <QtWidgets>
#include <QTabWidget>

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

private:
    QObject *m_pOwner;
};


#endif // QPROPPAGES_H
