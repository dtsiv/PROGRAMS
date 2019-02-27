#ifndef QLEDINDICATOR_H
#define QLEDINDICATOR_H

#include <QLabel>
#include <QMap>

#define QLEDINDICATOR_SIZE    96

#define QLEDINDICATOR_TIMEOUT 1000

class QTimer;

class QLedIndicator : public QLabel
{
	Q_OBJECT

public:
	enum QLedColor {qlc_gray,qlc_green,qlc_red,qlc_blue,qlc_yellow};
	enum QLedState {
        StateNum0=0,          // general gray
        StateNum1=1,          // general green
        StateNum2=2,          // general yellow
        StateNum3=3,          // general red
        StateNum4=4,          // general red blinking
        StateNum5=5,          // general blue
        StateReady,           // STATUS::rdy==true
        StateErrCon,          // m_pConnection error
        StateConnected,       // m_pConnection connected
        StateDisconnected,    // m_pConnection disconnected
        StateIrradiationOn,   // STATUS::emi==true
        StateOff,             // default LED indicator state
        StateError,           // reserved for error states
        StateDummy,           // used to throw exception
        TempBelow5,           // STATUS::Tgr<5
        TempOk,               // 5<STATUS::Tgr<60
        TempAbove60,          // 60<STATUS::Tgr<75
        TempAbove75           // 75<STATUS::Tgr
    };

	QLedIndicator(bool bBlinking=false, int iHeight=16, QWidget *parent=0, Qt::WindowFlags f=0);
	~QLedIndicator();
	void setColorForState(QLedIndicator::QLedState state);
    void setTemperature(int iTemp);
    void setState(QLedState qlsState);
    void setNumState(int iState);

protected slots:
	void onTimeout();
	void errorReport();

private:
	QMap<int,QPixmap*> m_qmPixmaps;
	QLedState m_qlsState;
	QTimer *m_pTimer;
	QPixmap *m_ppmCurrent;
};

#endif // QLEDINDICATOR_H
