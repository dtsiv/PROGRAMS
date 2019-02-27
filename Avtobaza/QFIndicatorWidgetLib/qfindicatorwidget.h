#ifndef QFINDICATORWIDGETLIB_H
#define QFINDICATORWIDGETLIB_H

#include <QtCore>
#include <QWidget>

#ifndef __linux
#include "codograms.h"
#else
#include "codograms_and_qavtctrl.h"
#endif

#include "qfindicatorwidget_global.h"

class QMutex;
class QDomDocument;
class QBoxLayout;
class TFIndicatorLine;
class TFIndicatorLineStyle;
class TRequestTread;
//******************************************************************************
//
//******************************************************************************
class QFINDICATORWIDGET_EXPORT QFIndicatorWidget : public QWidget
{
	Q_OBJECT

public:
	QFIndicatorWidget(QWidget * parent);
	~QFIndicatorWidget();
	Q_INVOKABLE QStringList about();
	void lock(void);
	void unlock(void);
	void rebuild(void) { resize(width(), height()); }
	Q_INVOKABLE void getPropPages(QList<QWidget *> * pwl, QWidget * pParent);
	Q_INVOKABLE void initialize(QDomDocument * pDomProp);
	Q_INVOKABLE void getConfiguration(QDomDocument * pDomProp);
	Q_INVOKABLE void ApplyChanges(QList<TFIndicatorLineStyle *> * plist);
	Q_INVOKABLE void traceTbl(PTRACELISTENTRY ptle, int iCou);
	Q_INVOKABLE void traceInfo(PTRACEINFOHEADER ptih);
	Q_INVOKABLE void traceInfoE(PTRACEINFOHEADER ptih);
	Q_INVOKABLE void sendInfoReuqest(int iId, double dFrom, double dTo);
	Q_INVOKABLE void AddPoit(PPOIT ppoit);
	Q_INVOKABLE void AddPoit(PPOITE ppoit);
	Q_INVOKABLE void AddPostt(PPOSTT ppt);
	Q_INVOKABLE void AddRaw16(PSCOPE16C pscope); 
	void clear(void);
	QList<TFIndicatorLine *> m_FLineList;
	void EmitFSelection(int id, double dFSel, double dBandSel, int iCPCount, double dAzFrom, double dAzTo, int iFlags);
    Q_INVOKABLE void updateFrSel(int iId, PFRSELECTION ps, int iCou);
	static int sizeofImp(PIMPINFO pii);
	static int sizeofStrob(PPOSTT ppt);
	static PIMPINFO nextImp(PIMPINFO pii);

private:
	QMutex m_Lock;
	QBoxLayout * m_pLayout;
	TRequestTread * m_pRequestTread;

public slots:

signals:
	void SendCommand(int iCommId, int TraceId, double dPar1, double dPar2);
	void SendFSelection(int id, double dFSel, double dBandSel, int iCPCount, double dAzFrom, double dAzTo, int iFlags);

public:
	bool m_bRequestsEnab;

protected:
};

#endif // QFINDICATORWIDGETLIB_H
