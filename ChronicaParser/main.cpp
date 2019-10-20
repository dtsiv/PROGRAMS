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

    // initialize random number generator. idum is ReadWrite
    idum=-1;
    NR::gasdev(idum);

    switch (omSelectedMode) {
        case mDataImport:
            if ((iErr=openDataFile())) {
                tsStdOut << "openDataFile() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mPOI20191016:
            if ((iErr=poi20191016())) {
                tsStdOut << "poi20191016() returned: " << iErr << endl;
                return 1;
            }
            closeDatabase();
            break;

        case mInterferenceSpectrum:
            if ((iErr=interferenceSpectrum())) {
                tsStdOut << "interferenceSpectrum() returned: " << iErr << endl;
                return 1;
            }
            break;

        case mInterferenceMap:
            if ((iErr=interferenceMap())) {
                tsStdOut << "interferenceMap() returned: " << iErr << endl;
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
