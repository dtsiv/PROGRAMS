#ifndef FINDICATORPROPPAGE_H
#define FINDICATORPROPPAGE_H

#include <QWidget>
#include <QColorDialog>
#include "ui_proppage.h"
class QFIndicatorWidget;
class TFIndicatorLineStyle;
#ifdef __linux
#include "codograms_and_qavtctrl.h"
#endif
#define STL_ADD		200
#define STL_REMOVE	201
//*****************************************************************************
//
//*****************************************************************************
class FIndicatorPropPage : public QWidget, public Ui::QFIndicatorPropPage
{
	Q_OBJECT

public:
	FIndicatorPropPage(QWidget * parent, QFIndicatorWidget * iparent);
	~FIndicatorPropPage();

public slots:
	void OnApplayChanges(PDWORD pEFl);
	void OnButtonBkgColorClicked(void);
	void OnCustomContextMenuRequest(QPoint pos);
	void OnButtonFontClicked(void);
	void OnFLineSelectionChanged(void);
	void OnButtonItemColorClicked(void);
	void OnCheckBoxShowChanged();
	void OnDensValueChanged(int iVal);
	void OnFormularTypeChanged(int iType);
	void OnIdEditingFinished(void);
	void OnRadioButtunClicked(void);
	void OnAzValueChanged(double dVal);

protected:

private:
	QFIndicatorWidget * m_pFIndicator;
	QList<TFIndicatorLineStyle *> m_StyleList;
	QMenu * m_pMenu;
	void AddStyle(int iRow);
	void RemoveStyle(int iRow);
	void DrawFLinesTbl(void);

public:

};
#endif //FINDICATORPROPPAGE
