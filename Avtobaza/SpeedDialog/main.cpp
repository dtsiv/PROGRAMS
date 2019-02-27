#include <QtGui/QApplication>

#include "speeddialog.h"
#include "qrmoconnection.h"
#include "qexceptiondialog.h"

class MyApplication : public QApplication {
public:
    MyApplication(int& argc, char ** argv) :
    QApplication(argc, argv) { }
    virtual ~MyApplication() { }

    // reimplemented from QApplication so we can throw exceptions in slots
    virtual bool notify(QObject * receiver, QEvent * event) {
        try {
            return QApplication::notify(receiver, event);
        } catch(std::exception& e) {
            showExceptionDialog(QString("Exception thrown: ")+ e.what());
        }
        return false;
    }
};

int main(int argc, char *argv[]) {
    MyApplication a(argc, argv);
    SpeedDialog w;
    w.show();
    return a.exec();
}


