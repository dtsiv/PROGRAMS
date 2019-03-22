#include "qspear.h"
#include <QApplication>
#if QT_VERSION >= 0x050100
#include <QtWidgets>
#endif
#include <QStyleFactory>

#include <Windows.h>

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
    SetErrorMode(SEM_FAILCRITICALERRORS) ;

    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    paths.append("sqldrivers");
    QCoreApplication::setLibraryPaths(paths);

    MyApplication a(argc, argv);

#if QT_VERSION >= 0x050100
    QApplication::setStyle(QStyleFactory::create("Fusion"));
#else
    QApplication::setStyle(new QCleanlooksStyle);
#endif
    QPalette pal;
    pal.setColor(QPalette::Window, QColor(70,90,90));
    pal.setColor(QPalette::Button, QColor(70,90,90));
    pal.setColor(QPalette::Highlight, QColor(80,110,110));
//        pal.setBrush(QPalette::Highlight, vlGrd);
    pal.setColor(QPalette::WindowText, QColor(220,250,250));
    pal.setColor(QPalette::ButtonText, QColor(220,250,250));
    pal.setColor(QPalette::Text, QColor(220,250,250));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::Base, QColor(30,50,50));
    pal.setColor(QPalette::AlternateBase, QColor(40,60,60));
    pal.setColor(QPalette::Light,QColor(30,50,50));
    pal.setColor(QPalette::Mid,QColor(180,210,210));
    QApplication::setPalette(pal);

    // QLocale::setDefault(QLocale::English);
    // setlocale( LC_ALL, "C" );
	QTranslator appTranslator;
	appTranslator.load(":/Resources/qspear_ru.qm", ".");
	qApp->installTranslator(&appTranslator);

	QSpear w;
	w.show();
	return a.exec();
}

