#include "routines.h"

/* Main program */
int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QTextStream tsStdOut(stdout);

    // xcgeev();
    // xdsbev();
    xdpbsv();
    // hamming();

    // tsStdOut << "Press any key" << endl;

    // QTextStream tsStdIn(stdin);
    // int aDum;
    // tsStdIn >> aDum;
    // QTimer::singleShot(0,&a,SLOT(quit()));
    // return a.exec();
    return 0;
}

