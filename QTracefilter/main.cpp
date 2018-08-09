#include "QTraceFilter.h"
#include <QtGui/QApplication>
#include "nr.h"

// members MatrixCach and LockCach are not supported any more
// Matrix::base_mat * Matrix::MatrixCach[MAXMATRIXSISE][MAXMATRIXSISE];
// CRITICAL_SECTION Matrix::LockCach;
SOMEKOFFS TdCord::sk;
double TdCord::dMajor = 6378245.0, TdCord::dMinor = 6356863.0;     // by Krasovsky default ellips
double TdCord::dFlat, TdCord::dESq, TdCord::dMagic = 1., TdCord::dDiscrPeriod = 0.1;
double TdCord::dOvrld = 0.25, TdCord::dBTKof = 7., TdCord::dEpsylonLimit = 15.51;
double TdCord::dRMaxinun = 500000.;
double TdCord::dRMinimum = 50000.;
double TdCord::dTimeGate = 0.5;
double TdCord::dAmpLimit = 8.;
double TdCord::dStrobMult2D = 3.2, TdCord::dStrobMult3D = 3.2;
double TdCord::dPreStrobMult2D = 3.2, TdCord::dPreStrobMult3D = 3.2;
int TdCord::iSectorsNum = 8;
int TdCord::iCashCount[6] = { 20, 20, 20, 20, 20, 20 };
bool TdCord::bStartAs3D = false;

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
    // QLocale::setDefault(QLocale::English);
    // setlocale( LC_ALL, "C" );
	QTraceFilter w;
	w.show();
	return a.exec();
}

//******************************************************************************
//
//******************************************************************************
double TPoiT::offset(PPOITE ppoit, int indx) {
	if(!bUseRxOffsets) return(0.);
	if (!QPoiModel::m_pPostCord) return(0.);

	PRXINFOE prx = &ppoit -> rx[indx];
	int i = 0;
	while(i < 5)
	{	POSTCORD::BASES_ * pb = &QPoiModel::m_pPostCord->bases[i];
		if(prx -> uMinuIndx == pb -> bMinuIndx + 1 && prx -> uSubtIndx == pb -> bSubtIndx + 1)
		{
//			DbgPrint(L"+++++ base %d-%d found", pb -> bMinuIndx, pb -> bSubtIndx);
			return(dRxOffsets[i]);
//			return(0.);
		}	
		i++;
	}
	return(0.);
}
