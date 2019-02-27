//================= slots =================
#include "qspear.h"
#include "codogramsa.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onCommand() {
    // sender Control
	QObject *pSender=sender();

    // save geo coordinates
    if (pSender==this->m_ppbSaveCoord) {
        QStringList qslCoord;
        qslCoord << m_pleAntB->text() << m_pleAntL->text() << m_pleAntH->text()
                 << m_pleCannonB->text() << m_pleCannonL->text() << m_pleCannonH->text();
        if (!m_pModel->setCoordinates(qslCoord))
            throw RmoException("m_pModel->setCoordinates() failed!");
        if (!m_pModel->writeCoordinates()) throw RmoException(m_pModel->m_qsErrorMessage);
        return;
    }

    // Buttons m_ppbRazvernuti and m_ppbExit do not change model
    if (pSender==m_ppbRazvernuti || pSender==m_ppbExit) {
        bool bMainWindow=true;
        if (pSender==m_ppbRazvernuti) bMainWindow=false;
        if (pSender==m_ppbExit) bMainWindow=true;

        for (int i=0; i<m_qlMainControls.size(); i++) {
            m_qlMainControls.at(i)->setEnabled(bMainWindow);
        }
        for (int i=0; i<m_qlWin1Controls.size(); i++) {
            m_qlWin1Controls.at(i)->setEnabled(!bMainWindow);
        }
        return;
    }
    // clear messages
    if (pSender==this->m_ppbClearMessages) {
        this->m_pteConsole->clear();
    }

    // server-related commands
    if (!m_pModel->isConnected()) return;
    // cannot send CTRL, MODE after initialization error
    if (m_pModel->bInitError()) return;
	if (pSender==this->m_ppbIrradiation) {
        bool bOn=m_ppbIrradiation->isChecked();
        m_pModel->setIrradiation(bOn);
		// Irradiation LED is controlled by CGT_STATUS
        // indicateIrr(bOn);
    }
	if (pSender==this->m_ppbFK) {
        if (!this->m_ppbIrradiation->isChecked()) {
            if (QMessageBox::question(this,tr("QuestionIrrOff"),tr("DoYouAgreeFK?"),QMessageBox::Cancel|QMessageBox::Ok)!=QMessageBox::Ok) return;
        }
        m_pModel->startFK();
    }
    if (pSender==this->m_ppbStrelba) {
        m_pModel->startStrelba();
        //== for expt!! ==================
        //#QByteArray *pba=m_pModel->getCodogramByType(CGT_STATUS);
        //#CG_STATUS *pCg=(CG_STATUS *)pba->data();
        //#pCg->Tgr=1;
        //#pCg->emi=1;
        //#pCg->rdy=1;
        //#pCg->enT=1;
        //#pCg->pp4_1=1;
        //#pCg->pp4_2=2;
        //#pCg->pp4_3=3;
        //#pCg->pp4_4=4;
        //#pCg->cs1=5;
        //#pCg->cs2=0;
        //#m_pteConsole->appendPlainText(QString(tr("Sending expt buffer type 0x%1")).arg(CGT_STATUS,0,16));
        //#m_pConnection->send(pba,CGT_STATUS);
        //#delete pba;
        //================================
    }
    if (pSender==this->m_ppbSvernuti) {
        m_pModel->svernuti();
    }
    if (pSender==this->m_ppbApplyFreq) {
        m_pModel->setFreq(this->m_pleFreq->text());
    }
    if (pSender==this->m_pslRotationSpeed) {
        int iSpeed=qMax(1,m_pslRotationSpeed->value());
        m_pslRotationSpeed->setValue(iSpeed);
        m_pqlRotSpeed->setText(QString("%1: %2").arg(tr("RotSpeed")).arg(iSpeed));
        m_pModel->setRotSpeed(iSpeed);
    }
    if (pSender==this->m_ppbSaveDirAS) {
        m_pModel->setASysDir();
    }
    if (pSender==this->m_ppbSaveDirCan) {
        m_pModel->setCannonDir();
    }
    if (pSender==m_ppbRotLeft || pSender==m_ppbRotRight
     || pSender==m_ppbRotUp   || pSender==m_ppbRotDown
     || pSender==m_ppbRotStop) {
         int iDir=-1;
         if (pSender==m_ppbRotLeft)  iDir=4;
         if (pSender==m_ppbRotUp)    iDir=1; 
         if (pSender==m_ppbRotDown)  iDir=2; 
         if (pSender==m_ppbRotRight) iDir=3; 
         if (pSender==m_ppbRotStop)  iDir=5; 
         if (iDir>=0) m_pModel->setRotDir(iDir);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onModelChanged(QSpearModel::ChangeFlags fWhat) {
    int iType=0;
    QByteArray *pba=0;
    if (fWhat == QSpearModel::flConnected && m_pModel->isConnected()) {
        bool bInitError=m_pModel->bInitError();
        bool bMainWinEnabled=!bInitError;
        for (int i=0; i<m_qlWin1Controls.size(); i++) {
            m_qlWin1Controls.at(i)->setEnabled(false);
        }
        for (int i=0; i<m_qlMainControls.size(); i++) {
            m_qlMainControls.at(i)->setEnabled(bMainWinEnabled);
        }
        m_ppbRazvernuti->setEnabled(true);
        m_pledAntennaSys->setState(QLedIndicator::StateConnected);
    }
    if (fWhat == QSpearModel::flConnected && !m_pModel->isConnected()) {
        for (int i=0; i<m_qlWin1Controls.size(); i++) {
            m_qlWin1Controls.at(i)->setEnabled(false);
        }
        for (int i=0; i<m_qlMainControls.size(); i++) {
            m_qlMainControls.at(i)->setEnabled(false);
        }
        m_pledAntennaSys->setState(QLedIndicator::StateDisconnected);
    }
    if (fWhat == QSpearModel::flCTRL) {
        if (m_pModel->bInitError()) return;
        iType=CGT_CTRL;
        pba=m_pModel->getCodogramByType(iType);
        m_pConnection->send(pba,iType);
    }
    if (fWhat == QSpearModel::flMODE) {
        if (m_pModel->bInitError()) return;
        iType=CGT_MODE;
        pba=m_pModel->getCodogramByType(iType);
        m_pConnection->send(pba,iType);
    }
    if (fWhat == QSpearModel::flAntPos) {
        iType=CGT_ANT_POS;
        pba=m_pModel->getCodogramByType(iType);
        m_pConnection->send(pba,iType);
    }
    if (fWhat == QSpearModel::flAntSens) {
        m_pqlASAzim->setText(m_pModel->qsAzim());
	    m_pqlASElev->setText(m_pModel->qsElev());
    }
    if (fWhat == QSpearModel::flIni) {
        if (m_pModel->bInitError()) return;
        iType=CGT_INI;
        pba=m_pModel->getCodogramByType(iType);
        m_pConnection->send(pba,iType);
    }
    if (fWhat & QSpearModel::flStatus) {
        pba=m_pModel->getCodogramByType(CGT_STATUS);
        CG_STATUS* pS= (CG_STATUS*)pba->data();
        if (fWhat & QSpearModel::flUD01)    m_pledUD01           ->setNumState(pS->ud);
        if (fWhat & QSpearModel::fl4PP01_1) m_pled4PP01_1        ->setNumState(pS->pp4_1);
        if (fWhat & QSpearModel::fl4PP01_2) m_pled4PP01_2        ->setNumState(pS->pp4_2);
        if (fWhat & QSpearModel::fl4PP01_3) m_pled4PP01_3        ->setNumState(pS->pp4_3);
        if (fWhat & QSpearModel::fl4PP01_4) m_pled4PP01_4        ->setNumState(pS->pp4_4);
        if (fWhat & QSpearModel::fl3PP01)   m_pled3PP01          ->setNumState(pS->pp3);
        if (fWhat & QSpearModel::flCS01_1)  m_pledCS01_1         ->setNumState(pS->cs1);
        if (fWhat & QSpearModel::flCS01_2)  m_pledCS01_2         ->setNumState(pS->cs2);
        if (fWhat & QSpearModel::flGG01)    m_pledGG01           ->setNumState(pS->gg);
        if (fWhat & QSpearModel::flSU01)    m_pledSU01           ->setNumState(pS->su);
		if (fWhat & QSpearModel::flTgr)     m_pledTemperature    ->setNumState(pS->Tgr); // UNUSED: setTemperature(pS->Tgr);
        if (fWhat & QSpearModel::flRdy)     indicateReady(pS->rdy);
        if (fWhat & QSpearModel::flEmi)     indicateIrr(pS->emi);
        if (fWhat & QSpearModel::flEnT)     m_pModel->startWork(pS->enT);
    }
    if (fWhat & QSpearModel::flRotate) {
        int iSpeed=m_pModel->rotSpeed();
        m_pqlRotSpeed->setText(QString("%1: %2").arg(tr("RotSpeed")).arg(iSpeed));
        m_pslRotationSpeed->setValue(iSpeed);
        iType=CGT_ROTATE;
        pba=m_pModel->getCodogramByType(iType);
        m_pConnection->send(pba,iType);
    }
	if (iType && pba) m_pteConsole->appendPlainText(QString(tr("Sending buffer type 0x%1 size %2."))
				.arg(iType,0,16).arg(pba->size()));
    if (pba) delete pba;
	return;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onConnected(QString sHost,int iPort) {
	QString qsMsg=QString(tr("Connected to host %1 port %2.")).arg(sHost).arg(iPort);
	m_pteConsole->appendPlainText(qsMsg);
	m_pqlStatusMsg->setText(qsMsg);
    m_pModel->setConnected(true);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onDisconnected() {
	QString qsMsg(tr("Disconnected"));
	m_pteConsole->appendPlainText(qsMsg);
	m_pqlStatusMsg->setText(qsMsg);
    m_pModel->setConnected(false);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onError(QAbstractSocket::SocketError iErrorCode, QString qsErrorString) {
     static QString QRmoConnection_messages[] = {
         tr("Connection refused"),
         tr("The remote host closed the connection"),
         tr("QRmoConnection: Not connected.")
     };
	m_pteConsole->appendPlainText(tr(qsErrorString.toLocal8Bit()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::indicateIrr(bool bOn) {
    m_pqlIrradiation->setText(bOn?tr("Irradiation on"):tr("Irradiation off"));
    m_pledIrradiation->setState(bOn?QLedIndicator::StateIrradiationOn:QLedIndicator::StateOff);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::indicateReady(bool bOn) {
    m_pqlReady->setText(bOn?tr("Ready"):tr("NotReady"));
    m_pledReady->setState(bOn?QLedIndicator::StateReady:QLedIndicator::StateOff);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onStartup() {
	if (sizeof(CG_ANT_SENSOR)!=24) throw RmoException("sizeof(CG_ANT_SENSOR) is not 24 bytes!");
	 // work timer
	m_pModel->startWork(false);
	QObject::connect(&this->m_workTimer,SIGNAL(timeout()),SLOT(onWorkTimeout()));
	m_workTimer.start(1000);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onWorkTimeout() {
	m_pqlWorkTime->setText(m_pModel->workTime());
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onAbout() {
	QDialog aboutDlg(this);
	QVBoxLayout vbLayout;
	QListWidget qlwVer;
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok);
	QObject::connect(&buttonBox,SIGNAL(accepted()),&aboutDlg,SLOT(close()));
	qlwVer.addItem(new QListWidgetItem(QString("Qt version %1").arg(QT_VERSION_STR)));
	qlwVer.addItem(new QListWidgetItem(QString("QSpear version %1.%2")
		.arg(QSPEAR_VER_MAJOR).arg(QSPEAR_VER_MINOR,2,10,QChar('0'))));
	qlwVer.addItem(new QListWidgetItem(QString("Codogramsa.h version %1.%2")
		.arg(CODOGRAMS_H_VER_MAJOR).arg(CODOGRAMS_H_VER_MINOR,2,10,QChar('0'))));
	vbLayout.addWidget(&qlwVer);
	vbLayout.addWidget(&buttonBox);
    aboutDlg.setLayout(&vbLayout);
	aboutDlg.setWindowTitle(tr("About"));
    aboutDlg.exec();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::onReceive(QByteArray *pba,int iType) {
    // debug output
    m_pteConsole->appendPlainText(QString(tr("Got buffer type 0x%1 size %2."))
				.arg(iType,0,16).arg(pba->size()));
    if (iType==CGT_TEXT) {
        m_pteConsole->appendPlainText(QString((QChar*)pba->data(),pba->size()/2));
    }
    // reject STATUS etc if not connected
    if (m_pModel->isConnected()) {
        m_pModel->recvCodogram(iType,pba);
    }
	delete pba;
	return;
}
