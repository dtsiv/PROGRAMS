POSTCORD journey
================
1. Introduction

    The relevant structures are

	typedef struct MAINCTRL_
	{   DWORD dwFlags;                  // флаги управления MAIN_FLAG_ХХХХ
		DWORD dwOStart;                 // кандидат в стартовые режимы
		DWORD dwONum;                   // число определенных режимов (макс 64)
		MAINCTRL_O o[64];               // массив структур режимов (программа обзора)
		unsigned __int64 quForvardTime;
		DWORD aUvsAddress[8];
		int iUvsPort[8];
		int iLocalUvsId;
		MAINCTRL_C c[24];
		DWORD dwChannelChangeMask[8];
		union {
			struct {
			USHORT usWbFrequ;
			short sFrequ;
			};
			DWORD dwFrequ;
		} ftbl[140];
		//=========================== MAINCTRL_P contains posts coords =========================================
		MAINCTRL_P p;
		MAINCTRL_A a[8];
		DWORD dwAttChangeMask;
		DWORD dwReserved0;
		DWORD dwRegisters[8];
		BYTE bPostEnableMask;
		BYTE bReserved[3];
		DWORD dwDcRegisters[4];
		DWORD dwReserved[10];
	} MAINCTRL, * PMAINCTRL;

	//============================== These two struct are IDENTICAL ============================================
	typedef struct MAINCTRL_P_                        typedef struct POSTCORD_
	{	GROUNDINFO positions[8];	                  {	GROUNDINFO positions[8];	  
		double dViewPointLat;                         	double dViewPointLat;
		double dViewPointLon;                         	double dViewPointLon;
		DWORD dwPosCount;                             	DWORD dwPosCount;
		struct                                        	struct BASES_
		{   BYTE bMinuIndx, bSubtIndx;                	{   BYTE bMinuIndx, bSubtIndx;
			USHORT usFlags;                           		USHORT usFlags;
			union {                                   		union {
				DWORD dwTimeOut;	                  			DWORD dwTimeOut;	
				DWORD dwFrGate;                       			DWORD dwFrGate;
				DWORD dwDurGate;                      			DWORD dwDurGate;
				float fAreEpsilon;                    			float fAreEpsilon;
			};                                        		};
			double dBaseLen;                          		double dBaseLen;
		} bases[5];                                   	} bases[5];
	} MAINCTRL_P, * PMAINCTRL_P;                      } POSTCORD, * PPOSTCORD;

2. Initialization

    QAvtAdu::m_pMainCtrl = (PMAINCTRL)m_fMainCtrl.map(0, sizeof(MAINCTRL));
    File name "mainctrl.cfg", location is C:\PS4NT\qbin:
	m_qsMainCtrlFilename = dir.absoluteFilePath(m_qsMainCtrlFilename);

3. The Journey

	void TPsCommandThread::run(void) { ... //==== qavtadu.cpp
		if(sendPostInfo()) return;
		...
			case WTYP_POSTINFOE:
				m_pAdu -> UpdatePostInfo((PPOSTCORD)m_hBuf -> lpData);
	}
	void QAvtAdu::dispatchCtlInput(char * buf, int iSize, int iType) { ...
		sendToClients((char *)m_pMainCtrl, sizeof(MAINCTRL), ADU_TYPE_MCTRL, m_hSndEvent)
	}
	bool TPsCommandThread::sendPostInfo(void) { //===== qavtadu.cpp =====
		PPOSTCORD ppi = (PPOSTCORD)m_hBuf -> lpData;
		m_hBuf -> dwType = WTYP_POSTINFOE;
		m_hBuf -> dwSize = sizeof(POSTCORD);
		m_pAdu -> lock();
		PMAINCTRL_P pp = &m_pAdu -> mainctl() -> p;
		int i = 0;
		while(i < 8)
		{	pp -> positions[i].iId = i;
			i++;
		}	
		pp -> dwPosCount = 8;
		memcpy(ppi, pp, m_hBuf -> dwSize);
		m_pAdu -> unlock();
		return(forward());
	}
	void TPsCommandThread::run(void) { ... //======= quvsclient.cpp ========
			if(m_pInChan -> Get(&m_hBuf)) ...
				case WTYP_POSTINFOE:
					m_pUvsClient -> setPostInfo((PPOSTCORD)m_hBuf -> lpData);
					if(forward()) return;
					break;
	}
	POSTCORD QUvsClient::m_PostCord; //====== quvsclient.h ======
	void QUvsClient::setPostInfo(PPOSTCORD pPostCord) { //======= quvsclient.cpp ========
		memcpy(&m_PostCord, pPostCord, sizeof(m_PostCord));
	}
	void TPsCommandThread::run(void) { ...  //====== qpspoi.cpp ======
		case WTYP_POSTINFOE:
			m_pPoi -> setPostInfo((PPOSTCORD)m_hBuf -> lpData);
			if(forward()) return;
	}
    void QPsPoi::setPostInfo(PPOSTCORD ppi) { //====== qpspoi.cpp ======
		m_PostCount = ppi -> dwPosCount;
		m_postBlh[0].dHei = 0.;
		m_postBlh[0].dLat = ppi -> dViewPointLat * DEG_TO_RAD;
		m_postBlh[0].dLon = ppi -> dViewPointLon * DEG_TO_RAD;
		m_iStrobTimeOut = ppi -> bases[0].dwTimeOut;
		m_iFrGate = ppi -> bases[1].dwFrGate;
		m_dDurGate = 1.e-5 * (double)ppi -> bases[2].dwDurGate;
		m_AreEpsilon = (double)ppi -> bases[3].fAreEpsilon;
		if(m_AreEpsilon <= 0.) m_AreEpsilon = 1.;
		m_LightVelo = 299792458. / sqrt(m_AreEpsilon * m_AreMju);
		int i = 1;
		while(i < 9)
		{	PBLH pblh = &ppi -> positions[i - 1].blh;
			m_postBlh[i].dLat = pblh -> dLat * DEG_TO_RAD;
			m_postBlh[i].dLon = pblh -> dLon * DEG_TO_RAD;
			m_postBlh[i].dHei = pblh -> dHei;
			LBH lbh;
			lbh.dLat = m_postBlh[i].dLat;
			lbh.dLon = m_postBlh[i].dLon;
			lbh.dHei = m_postBlh[i].dHei * 0.001;
			Blh2Xyz(lbh, &m_postXyz[i - 1]);
			i++;
		}
		i = 0;
		while(i < 5)
		{	POSTCORD::BASES_ * pb = &ppi -> bases[i];
			m_bMinuIndx[i] = pb -> bMinuIndx;
			m_bSubtIndx[i] = pb -> bSubtIndx;
			m_dwBaseLen[i] = (DWORD)(1.e9 * pb -> dBaseLen / m_LightVelo);
			DbgPrint(L"Q-PsPoi: got base %ld len=%ld ns, mindx=%d sindx=%d\n",
				i, m_dwBaseLen[i], m_bMinuIndx[i], m_bSubtIndx[i]);
			m_difs[i].clear();
			i++;
		}
		...
	}
	int QUvsClient::SendPoit(PPOITE ppte) {
		ppte -> blh[0].dLat = m_PostCord.positions[m_aAduAddress.iLocalId].blh.dLat * DEG_TO_RAD;
		ppte -> blh[0].dLon = m_PostCord.positions[m_aAduAddress.iLocalId].blh.dLon * DEG_TO_RAD;
		ppte -> blh[0].dHei = 0.;
		int i = 0;
		while(i < 8) //============== At this point numbering was changed: (Post->index)=(4->5) and the like ==============
		{	ppte -> blh[i + 1].dLat = m_PostCord.positions[i].blh.dLat * DEG_TO_RAD;
			ppte -> blh[i + 1].dLon = m_PostCord.positions[i].blh.dLon * DEG_TO_RAD;
			ppte -> blh[i + 1].dHei = m_PostCord.positions[i].blh.dHei * 0.001;
			i++;
		}
		...
	}

4. The BLH story
	BLH: dLat,dLon(DEG); dHei(m)
	LBH: dLat,dLon(RAD); dHei(km)

5. RXINFOE meaning (this is part of POITE)
	typedef struct RXINFOE_ {   
		unsigned short uMinuIndx, uSubtIndx;  //========== Гео координаты постов (ссылки к blh)
		int iIndex;             // номер имп. в пачке от 0
		unsigned long uT;       // uT - Временное смещение от начала СЛ в нс
		union {
			double Az;				// Азимут а радианах
			double D;				// рх в м
		};
		union {
			double sAz;				// СКО азимута в рад
			double sD;				// СКО рх в м
		};
		double dAmp;			// Амплитуда дБ 
		double dTau;			// Длит имп мкс
		double dF;				// Цент. частота Мгц
		DWORD dwFrSigma2;
		DWORD dwStrobEQuad;
		DWORD dwPeriod;			// период в пачке в нс
		USHORT usImpCount;		// число имп. в пачке
		short sFreq0, sFreq1;	// KHz от центральной
		USHORT usStrobE;
		union {
		struct {
			DWORD D4_C1_UBD;
			DWORD DP1;
			DWORD DP2;
			DWORD DP3; };
		BYTE bModeSData[16]; }; 
		USHORT usSifMode;
		USHORT usFlags;
		int iNTacan;
	} RXINFOE, * PRXINFOE;

	//======== p00 p01 p10 p11 p20 p21 explained ========
	if(prx -> p00 == prx -> p10) {	//                 
		p0 = prx -> p00;            //              /------------- p01=p1 (MinuIndx)
		p1 = prx -> p01;            //   p00=p10   /   
		p2 = prx -> p11;            //          (*)
		D0 = prx -> D0;             //             \
		D1 = prx -> D1;             //              \------------- p11=p2 (SubtIndx)
	}
    //-----------------------------------------------------------------------
	// D0 - path difference R(MinuIndx,tg)-R(SubtIndx,tg) for pulse 0
	// D1 - path difference R(MinuIndx,tg)-R(SubtIndx,tg) for pulse 1
	// D2 - path difference R(MinuIndx,tg)-R(SubtIndx,tg) for pulse 2
	// p00= &blh[ppoit -> rx[0].uMinuIndx];
	// p01= &blh[ppoit -> rx[0].uSubtIndx];
	// p10= &blh[ppoit -> rx[1].uMinuIndx];
	// p11= &blh[ppoit -> rx[1].uSubtIndx];
	// p20= &blh[ppoit -> rx[2].uMinuIndx];
	// p21= &blh[ppoit -> rx[2].uSubtIndx];
    //-----------------------------------------------------------------------
	//=========== Init type A ============================================== Init type B ================================
	TPoiT::TPoiT(unsigned __int64 uqTime,                       TPoiT::TPoiT(PPOITE ppoit) {   
	   double dLat, double dLon) {   m_uqTime = uqTime;         	int i=0;  m_uqTime = ppoit -> uqTlock;
		tag.dLat = dLat;                                            memcpy(&blh, &ppoit -> blh, sizeof(blh));
		tag.dLon = dLon;                                            bXYValid = false;	m_bAlterXY = false;	m_bAmbigous = false;	bInvalid = true;	tag.dHei = 5000.;
		tag.dHei = 0.;                                          	m_Sector = ppoit -> iSector;	m_uFlags = ppoit -> uFlags;    rx = NULL;    Count = 0;
		blh[0] = tag;                                           	int iImpIndx = 0;
		blh[0].dLat -= M_PI / 1800.;                            	while(i < ppoit -> Count) {	
		blh[1] = tag;                                           	    if(ppoit -> rx[i++].iIndex == iImpIndx) continue;
		blh[1].dLat -= M_PI / 1800.;                            		iImpIndx++;
		blh[2] = tag;                                           	}
		blh[2].dLon += M_PI / 1800.;                                if(ppoit -> rx[ppoit -> Count - 1].iIndex != iImpIndx) {   
		blh[3] = tag;                                           	    DbgPrint(L"Q-PsVoi: Error: Invalid poit array structure RxCount=%d", ppoit -> Count);
		blh[3].dLon -= M_PI / 1800.;                                    return;
		blh[4] = tag;                                               }
		blh[4].dLat += M_PI / 1800.;                            	Count = iImpIndx + 1; rx = new RXPOINT[Count]; PRXPOINT prx = &rx[0]; i = 0; iImpIndx = 0;
		blh[4].dLon -= M_PI / 1800.;                            	while(i < ppoit -> Count)
		bXYValid = false;                                       	{	prx -> dTimeOffs = 1.e-9 * (double)(long)ppoit -> rx[i].uT; // uT in nanoSec
		bInvalid = false;                                               prx -> dAmp = ppoit -> rx[i].dAmp;
		m_bAmbigous = false;                                    		prx -> p00 = &blh[ppoit -> rx[i].uMinuIndx];
		m_bAlterXY = false;                                             prx -> p01 = &blh[ppoit -> rx[i].uSubtIndx];
		m_Sector = 0;                                                   prx -> D0 = ppoit -> rx[i].D + offset(ppoit, i);
		m_uFlags = POIT_FLAGS_BLH_VALID                                 prx -> sD0 = dMagic * max(ppoit -> rx[i].sD,  0.0001);
		  | POIT_FLAGS_RX_VALID | POIT_FLAGS_VIEWP_VALID;               prx -> p10 = NULL; prx -> p11 = NULL; prx -> D1 = 0.; prx -> sD1 = VERYLARGEAMOUNT;
		Count = 1;                                                      prx -> p20 = NULL; prx -> p21 = NULL; prx -> D2 = 0.; prx -> sD2 = VERYLARGEAMOUNT;
		rx = new RXPOINT[Count];                                		if(++i >= ppoit -> Count) break;
		rx -> dTimeOffs = 0.;                                           if(ppoit -> rx[i].iIndex == iImpIndx)
		rx -> p00 = &blh[1];                                            {   prx -> p10 = &blh[ppoit -> rx[i].uMinuIndx];
		rx -> p01 = &blh[2];                                                prx -> p11 = &blh[ppoit -> rx[i].uSubtIndx];
		rx -> D0 = 0.;                                                      prx -> D1 = ppoit -> rx[i].D + offset(ppoit, i);
		rx -> sD0 = 0.05;                                                   prx -> sD1 = dMagic * max(ppoit -> rx[i].sD,  0.0001);
		rx -> p10 = &blh[1];                                    			if(ppoit -> rx[i].dAmp < prx -> dAmp) prx -> dAmp = ppoit -> rx[i].dAmp;
		rx -> p11 = &blh[3];                                    			if(++i >= ppoit -> Count) break;
		rx -> D1 = 0.;                                                  }
		rx -> sD1 = 0.05;                                               if(ppoit -> rx[i].iIndex == iImpIndx)
		rx -> p20 = &blh[1];                                            {   prx -> p20 = &blh[ppoit -> rx[i].uMinuIndx];
		rx -> p21 = &blh[4];                                                prx -> p21 = &blh[ppoit -> rx[i].uSubtIndx];
		rx -> D2 = 0.;                                                      prx -> D2 = ppoit -> rx[i].D + offset(ppoit, i);
		rx -> sD2 = 0.05;                                                   prx -> sD2 = dMagic * max(ppoit -> rx[i].sD,  0.0001);
	}                                                           			if(ppoit -> rx[i].dAmp < prx -> dAmp) prx -> dAmp = ppoit -> rx[i].dAmp;
                                                                			if(++i >= ppoit -> Count) break;
	                                                                    }
	                                                            		while(ppoit -> rx[i].iIndex == iImpIndx) if(++i >= ppoit -> Count) break;
	                                                            		prx++; iImpIndx++;
	                                                            	}
	                                                                bInvalid = false;
	                                                            }
//===================================== THE Point where POITE::RXINFOE::D gets its value ================================================
        ...
        PIMPMATCHTBL pm = ps -> m_MatchList.at(i++);	
		...
	    while(j < 5) {	//===== qpspoi.cpp ======
		int iMinuIndx = (int)m_pPoi -> m_bMinuIndx[j];	int iSubtIndx = (int)m_pPoi -> m_bSubtIndx[j];
		if(pm -> indx[iMinuIndx] == -1 || pm -> indx[iSubtIndx] == -1) { j++; continue; }
		PIMPINFO pi0 = ps -> impulses(iMinuIndx)[pm -> indx[iMinuIndx]]; PIMPINFO pi1 = ps -> impulses(iSubtIndx)[pm -> indx[iSubtIndx]];
		double dis0 = ps -> distances(iMinuIndx)[pm -> indx[iMinuIndx]]; double dis1 = ps -> distances(iSubtIndx)[pm -> indx[iSubtIndx]];
		...
		prx -> uT = uT;		prx -> iIndex = 0;		prx -> uMinuIndx = iMinuIndx + 1;		prx -> uSubtIndx = iSubtIndx + 1;
		prx -> dF = 0.001 * (double)pi0 -> iFreq;		double dTrash = 1.;
		if(pi0 -> usTrashHold) dTrash = (double)pi0 -> usTrashHold;
		if(pi0 -> iAmp > 0) prx -> dAmp = 20. * log10((double)pi0 -> iAmp / dTrash);	else prx -> dAmp = 0.;
		prx -> dTau = 0.01 * (double)pi0 -> iDur;		prx -> sD = 70.;		if(prx -> dAmp > 10.) prx -> sD = 50.;
		if(prx -> dAmp > 20.) prx -> sD = 20.;
		prx -> D = 1.e-9 * (double)((int)pi0 -> uT - (int)pi1 -> uT) * m_pPoi -> m_LightVelo; 
		if(dis0 != -1. && dis1 != -1.) {
			double dif = (dis0 - dis1) - prx -> D;
			if(fabs(dif) < 500.) m_pPoi -> m_difs[j] = dif;
		}
		if(m_pPoi -> m_iUseOffsets) prx -> D += (double)m_pPoi -> m_difs[j];
		prx++;		ppoit -> Count++;		j++;		if(ppoit -> Count > 2) break;
	}

	
