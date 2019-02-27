#include <QtGui>
#include <QApplication>
#include "chantblviewer.h"

// ----------------------------------------------------------------------
int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    ChanTblViewer   wgt;
	wgt.setWindowIcon(QIcon(":/QT.ico"));
    wgt.show();
	
    return app.exec();
}
