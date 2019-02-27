#include "stdafx.h"
#include "qcprmo.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068)
#endif
//*****************************************************************************
//
//*****************************************************************************
AboutDlg::AboutDlg(QCPRmo * parent) : QDialog((QWidget *)parent)
{
	setupUi(this);

    QStringList qsl = parent -> localctrlqml() -> about();
    qsl += parent -> findicator() -> about();
    qsl += parent -> console() -> about();

	listWidgetVer -> clear();
	listWidgetVer -> addItem(new QListWidgetItem("Qt 4.7.2"));
	QString qsVer;
#pragma GCC diagnostic ignored "-Wformat"
    qsVer.sprintf("Cross-platform RMO version %ld.%ld beta by Tristan 2016", QCPRMO_VERSION_MAJOR, QCPRMO_VERSION_MINOR);
	listWidgetVer -> addItem(new QListWidgetItem(qsVer));
	qsVer.sprintf("Codograms version %ld.%ld", CODOGRAMS_H_VER_MAJOR, CODOGRAMS_H_VER_MINOR);
#pragma GCC diagnostic pop
	listWidgetVer -> addItem(new QListWidgetItem(qsVer));
	int i = 0;
	while(i < qsl.count())
	{	QString qs = qsl.at(i++); 
		listWidgetVer -> addItem(new QListWidgetItem(qs));
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

