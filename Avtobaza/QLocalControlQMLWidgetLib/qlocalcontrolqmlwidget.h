#ifndef QLOCALCONTROLQML_H
#define QLOCALCONTROLQML_H

#include <QtCore>
#include <QWidget>

#ifdef __linux
    #include "qavtctrl_linux.h"
    #include "codograms_linux.h"
#else
    #include "qavtctrl.h"
    #include "codograms.h"
#endif

#include "qlocalcontrolqmlwidget_global.h"

#if MSVC
#include <windows.h>
#endif

class QDomDocument;
class QLocalControlQMLWidgetPrivate;
class QLOCALCONTROLQMLWIDGET_EXPORT QLocalControlQMLWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QLocalControlQMLWidget(QWidget *parent = 0);
	~QLocalControlQMLWidget();

protected:
	// to enable private class hierarchy
	QLocalControlQMLWidget(QLocalControlQMLWidgetPrivate /*must be &&*/ &d, 
							QWidget *parent);

public:
     // RMO delegates, they should comply with other widgets' API
	QStringList about();
    Q_INVOKABLE QString version();
    Q_INVOKABLE void getPropPages(QList<QWidget *> * pwl, QWidget * pParent);
    Q_INVOKABLE void initialize(QDomDocument * pDomProp);
    Q_INVOKABLE void getConfiguration(QDomDocument * pDomProp);
	 // functions called when codograms arrive
    Q_INVOKABLE void receiveLocalCtrlInfoEx(PLOCALCTRLSTRUCTEX pCtl, bool bAskForConfirmation=true);
    Q_INVOKABLE void receiveFrequenciesTable(PFREQUTBL pft,bool bAskForConfirmation=true);
	 // QML setup
    Q_INVOKABLE void setPathToQml(const QString &pathToQml);
    Q_INVOKABLE QString getPathToQml();
	 // Hotkey mgmt - this maybe changed to application-wide hotkeys
    Q_INVOKABLE void setHotKey(int i, QString hk);
    Q_INVOKABLE QString getHotKey(int i);

signals:
	 // signal used to send codogram when user changes parameter
    void SendCommand(char *, int, int);
    void uiParamsChanged();

protected:
	QLocalControlQMLWidgetPrivate * const d_ptr;

private:
	Q_DECLARE_PRIVATE(QLocalControlQMLWidget)
	Q_DISABLE_COPY(QLocalControlQMLWidget)
};

#endif // QLOCALCONTROLQML_H

