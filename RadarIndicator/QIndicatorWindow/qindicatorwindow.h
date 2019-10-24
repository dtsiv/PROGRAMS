#ifndef QIndicatorWindow_H
#define QIndicatorWindow_H

#include <QMainWindow>
#include <QSplashScreen> 
#include <QtGui>
#include <QtWidgets>
#include <QGLWidget>

#include "ui_qindicatorwindow.h"

class QIndicatorWindow : public QMainWindow
{
	Q_OBJECT

public:
    QIndicatorWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~QIndicatorWindow();

    Ui::QIndicatorWindowClass ui;
};

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

#endif // QIndicatorWindow_H
