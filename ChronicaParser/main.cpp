#include <QCoreApplication>
using namespace std;

#include "sqlmodel.h"
#include "poi.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    int iErr;
    QTextStream tsStdOut(stdout);

    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
    if (!db.isValid()) tsStdOut << "addDatabase failed" << endl;

    if ((iErr=readSettings())) {
        tsStdOut << "readSettings() returned: " << iErr << endl;
        return 1;
    }

    switch (omSelectedMode) {
        case mDataImport:
            if ((iErr=openDataFile())) {
                tsStdOut << "openDataFile() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mPrimaryProc:
            if ((iErr=poi())) {
                tsStdOut << "poi() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mPOI20190409:
            if ((iErr=poi20190409())) {
                tsStdOut << "poi20190409() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mJustDoppler:
            if ((iErr=justDoppler())) {
                tsStdOut << "justDoppler() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mPrimaryProcRaw:
            if ((iErr=poiRaw())) {
                tsStdOut << "poiRaw() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mPrimaryProcNoncoher:
            if ((iErr=poiNoncoher())) {
                tsStdOut << "poiNoncoher() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mSignalPlot:
            if ((iErr=plotSignal())) {
                tsStdOut << "plotSignal() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mSignalPlot3D:
            if ((iErr=plotSignal3D())) {
                tsStdOut << "plotSignal3D() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        default:
            tsStdOut << "unknown operation" << endl;
    }

    tsStdOut << "Press any key" << endl;

    QTextStream tsStdIn(stdin);
    int aDum;
    tsStdIn >> aDum;
    QTimer::singleShot(0,&a,SLOT(quit()));
    return a.exec();
}
