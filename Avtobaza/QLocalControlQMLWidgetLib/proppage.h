#ifndef PROPPAGE_H
#define PROPPAGE_H

#include <QWidget>
#include <QFileDialog>
#include <QDir>

#include "ui_proppageqml.h"
#include "qlocalcontrolqmlwidget.h"
class PropPage : public QWidget,public Ui::PropPageQML
{
	Q_OBJECT

public:
	PropPage(QWidget *parent, QLocalControlQMLWidget * iparent);
	~PropPage();

public slots:
	void OnApplayChanges(PDWORD pEFl);
    void OnPathToQmlButtonClicked(void);

private:
	QLocalControlQMLWidget * m_pLocalControlQMLWidget;
	
};

#endif // PROPPAGE_H
