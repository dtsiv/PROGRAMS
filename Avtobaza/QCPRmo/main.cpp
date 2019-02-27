#include "stdafx.h"
#include <QtGui/QApplication>
#include <QtDebug>

#include "qcprmo.h"
#include "rmoexception.h"
#include "qexceptiondialog.h"
#include "initialize.h"

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
      showExceptionDialog(QString("Exception thrown: ") + e.what());
    }
    return false;
  }
};

int main(int argc, char *argv[]) {
    MyApplication a(argc, argv);
    InitializeForm * initform = new InitializeForm(NULL);
    initform -> show();
    a.processEvents();
    QCPRmo w(a.arguments(), initform);
    w.show();
    delete initform;
    return a.exec();
}


