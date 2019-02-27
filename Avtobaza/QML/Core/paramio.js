/*================================================================================
 *  functions used to do parameters IO
 *================================================================================*/
 
// this is not stateless library 
// .pragma library

//================ Map parameter name in UI onto key of programsListModel.params hash ====================
function mapParamName(paramNameSrc) {
	// return paramNameSrc argument by default
	var paramName = paramNameSrc;
	
	// define mapping for special cases
	if (paramNameSrc == "freqFromHB") paramName = "freqFrom";
	if (paramNameSrc == "freqFromLB") paramName = "freqFrom";
	if (paramNameSrc == "freqToHB")   paramName = "freqTo";
	if (paramNameSrc == "freqToLB")   paramName = "freqTo";
	
    // UI buttons sector0,...,sector7,allSectors are mapped on paramName="sectors"
    if (~paramNameSrc.indexOf("sector") || paramNameSrc=="allSectors") {
        paramName = "sectors";
	}
	
    // UI buttons attenuator0,...,attenuator7,allattenuators are mapped on paramName="attenuators"
    if (~paramNameSrc.indexOf("attenuator") || paramNameSrc=="allAttenuators") {
        paramName = "attenuators";
	}
	
	// UI buttons band3, band10, band20, band40
    if (~paramNameSrc.indexOf("band")
    // reserved for future use
	  && paramNameSrc != "band3") {
        paramName = "band";
	}
	
	// return the resulting key for programsListModel.params hash
    return paramName;
}

//================ Apply a new value of single parameter "name" in sub-program progIdx ====================
function applyParam(progIdx,paramNameSrc) {
	var value = programsListModel.get(progIdx)[paramNameSrc];
	var reg;
	var paramName = mapParamName(paramNameSrc);
	
	var curParamDef=programsListModel.params[paramName];
	if (curParamDef==null) {
		// cppModel.msg("In applyParam: curParamDef==null: "+paramName+" "+value);
		return true;  // currently, "name", "enabled" are not keys of curParamDef, no problem!
	}
	if (paramNameSrc == "freqFromHB") {
		reg   = programsListModel.get(progIdx)["freqFromLB"];
		reg   = Math.round(parseFloat(reg));        // reg: LB
		value = Math.round(parseFloat(value)/10.0); // param: HB
		value &= 0x0000FFFF;
		reg   <<=16;
		reg   &= 0xFFFF0000;
		value |= reg;
	}
	if (paramNameSrc == "freqFromLB") {
		reg   = programsListModel.get(progIdx)["freqFromHB"];
		reg   = Math.round(parseFloat(reg)/10.0); // reg: HB
		value = Math.round(parseFloat(value));    // param: LB
		value <<=16;
		value &= 0xFFFF0000;
		reg   &= 0x0000FFFF;
		value |= reg;
	}
	if (paramNameSrc == "freqToHB") {
		reg   = programsListModel.get(progIdx)["freqToLB"];
		reg   = Math.round(parseFloat(reg));        // reg: LB
		value = Math.round(parseFloat(value)/10.0); // param: HB
		value &= 0x0000FFFF;
		reg   <<=16;
		reg   &= 0xFFFF0000;
		value |= reg;
	}
	if (paramNameSrc == "freqToLB") {
		reg   = programsListModel.get(progIdx)["freqToHB"];
		reg   = Math.round(parseFloat(reg)/10.0); // reg: HB
		value = Math.round(parseFloat(value));    // param: LB
		value <<=16;
		value &= 0xFFFF0000;
		reg   &= 0x0000FFFF;
		value |= reg;
	}
	if (~paramNameSrc.indexOf("sector")) return; // use allSectors instead
    if (~paramNameSrc.indexOf("attenuator")) return; // use allSectors instead
	// construct 8-bit value from individual bits
	if (paramNameSrc == "allSectors") {
		value=0;
		for (var i=0; i<8; i++) {
			var sectorName="sector"+i;
			var bit=programsListModel.get(progIdx)[sectorName];
			value|=(bit<<i);
		}
    }
	// construct 8-bit value from individual bits
	if (paramNameSrc == "allAttenuators") {
		value=0;
		for (var i=0; i<8; i++) {
			var attenuatorName="attenuator"+i;
			var bit=programsListModel.get(progIdx)[attenuatorName];
			value|=(bit<<i);
		}
    }
	// construct code of frequency band
	if (paramName == "band") {
		value = 
		    programsListModel.get(progIdx)["band40"]?0x00:0
		   +programsListModel.get(progIdx)["band20"]?0x01:0
		   +programsListModel.get(progIdx)["band10"]?0x02:0
		 // reserved for future use: programsListModel.get(progIdx)["band3"]?0x04:0
			;
    }
	// construct code of strob duration
	if (paramName == "analysis") {
        value = 0
		    +  (+value==5)   ?0x00:0
		    +  (+value==10)  ?0x01:0
		    +  (+value==50)  ?0x02:0
		    +  (+value==200) ?0x04:0;
	}	
    // write result to register/ftbl
	var mask     = curParamDef.mask[progIdx];
	var shift    = curParamDef.shift[progIdx];
	var type     = curParamDef.type[progIdx];
	var index    = curParamDef.index[progIdx];
	var isScalar = curParamDef.isScalar[progIdx];
	if (type==PARAM_TYPE_REGISTER)
		cppModel.setRegisterValue(index,value,mask,shift);
	if (type==PARAM_TYPE_FREQTBL)
		cppModel.setFrequencyValue(index,value,mask,shift);
	return true;
}

//================ Apply all parameters in sub-program progIdx ====================
function applyAllParams(progIdx) {
	// verify scan value
	if (isParamEnabled(progIdx,"scan")) {
        var allowedValues=[0x00,0x01,0x02,0x04,0x08,0x10];
        if (!~allowedValues.indexOf(+getParam(progIdx,"scan"))) return false;
	}
	// verify strob duration
	if (isParamEnabled(progIdx,"analysis")) {
        var allowedValues=[5,10,50,200];
        if (!~allowedValues.indexOf(+getParam(progIdx,"analysis"))) return false;
	}
	var freqFromHB = parseFloat(getParam(progIdx,"freqFromHB"));
	var freqToHB   = parseFloat(getParam(progIdx,"freqToHB"));
	var freqFromLB = parseFloat(getParam(progIdx,"freqFromLB"));
	var freqToLB   = parseFloat(getParam(progIdx,"freqToLB"));
	if (isParamEnabled(progIdx,"band40")
	&&  isParamEnabled(progIdx,"band20")
	&&  isParamEnabled(progIdx,"band10")
	// reserved for future use: && isParamEnabled(progIdx,"band3")
	&&  isParamEnabled(progIdx,"freqFromHB")
	&&  isParamEnabled(progIdx,"freqToHB") ) { // check consistency of input parameters
		var band=[];
		// reserved for future use: if (getParam(progIdx,"band3"))  band.push(3);
		if (getParam(progIdx,"band10")) band.push(10);
		if (getParam(progIdx,"band20")) band.push(20);
		if (getParam(progIdx,"band40")) band.push(40);
		if (band.length!=1) {
			cppModel.msg("In applyAllParams: invalid band");
			return false;
		}
		// now band[0] is one of these values 3,10,20,40
		band=band[0];
		var nSteps=Math.round(1.0*(freqToHB-freqFromHB)/band);
		if (nSteps<=0 || nSteps>6000) 	{
			cppModel.msg("In applyAllParams: nSteps<=0 || nSteps>6000");
			return false;
		}
		if (freqFromHB < 2000 || freqToHB   > 18000 || freqFromLB < 200) {
			cppModel.msg("In applyAllParams: freqFromHB,freqToHB,freqFromLB invalid");
			return false;
		}
		freqToLB=(freqFromLB + band*nSteps);
		if (freqToLB   > 2000 ) {
			cppModel.msg("In applyAllParams: freqToLB invalid");
			return false;
		}
		// cppModel.msg(" freqToHB="+freqToHB+" freqFromHB="+freqFromHB+" band="+band);
		// cppModel.msg("setting freqToLB="+freqToLB+" nSteps"+nSteps+" nSteps"+nSteps+" band="+band);
		programsListModel.setProperty(progIdx,"freqToLB", freqToLB);
	}
	var obj = programsListModel.get(progIdx);
	for (var paramName in obj) {
        if (~paramName.indexOf("sector")) continue; // use allSectors instead
        if (~paramName.indexOf("attenuator")) continue; // use allSectors instead
		if (!applyParam(progIdx,paramName)) {
			cppModel.msg("In applyAllParams: applyParam failed, paramName="+paramName);
			return false;
		}
	}
	return true;
}

//================= This function is called on Apply button click =================
function applyAllParamsAllSubprograms() {
	// loop over subprograms
	var programsCount=programsListModel.count;
	for (var progIdx=0; progIdx<programsCount; progIdx++) {
		if (!applyAllParams(progIdx)) {
			cppModel.msg("applyAllParams(progIdx) failed: "+progIdx);
			return false;
		}
	}
	// set parameter programsListModel.params["current_modes_mask"] if it is defined
	applyCurrentModesMask();
	// send codogram with updated structure APPLAYLOCALCTRLEX (codogram WTYP_CHANGEMODEEX)
	cppModel.submitLocalCtrlEx();
	// send codograms with updated frequency table elements m_pFtbl->freq[freqIdx].dwFrequ (codogram WTYP_FREQUCHANGE)
	for (var paramName in programsListModel.params) {
		var curParamDef=programsListModel.params[paramName];
		if (curParamDef == null) continue;
		var arrType=curParamDef["type"];
		var arrIndex=curParamDef["index"];
		if (arrType.length!=programsCount || arrIndex.length!=programsCount) continue;
		for (var i=0; i<programsCount; i++) {
			if (arrType[i]!=PARAM_TYPE_FREQTBL) continue; // continue inner loop
			// for subprogram i submit new value of frequency table at freqIdx=arrIndex[i]
			cppModel.submitFrequTblAtIdx(arrIndex[i]);
		}
	}
	return true;
}

//================ Apply all parameters in all sub-programs ====================
function applyCurrentModesMask() {
	var currentModesMaskParamDef=programsListModel.params["current_modes_mask"];
	if (currentModesMaskParamDef==null) return;
	var programsCount=programsListModel.count;
	if (programsCount>32) return; // only got 32bit mask (one register at max)
	var currentModesMask = 0;
	for (var i=0; i<programsCount; i++) {
		currentModesMask |= (1<<i)*(+programsListModel.get(i)["enabled"]);
	}
	var mask     = currentModesMaskParamDef.mask[0];
	var shift    = currentModesMaskParamDef.shift[0];
	var type     = currentModesMaskParamDef.type[0];
	var index    = currentModesMaskParamDef.index[0];
	var isScalar = currentModesMaskParamDef.isScalar; // "current_modes_mask" can only be scalar parameter
	if (!isScalar) return;
	if (type==PARAM_TYPE_REGISTER)
		cppModel.setRegisterValue(index,currentModesMask,mask,shift);
	if (type==PARAM_TYPE_FREQTBL)
		cppModel.setFrequencyValue(index,currentModesMask,mask,shift);
}

//================ restore parameter values from cppModel ====================
function isParamEnabled(progIdx,paramNameSrc) {
	
    // Map parameter name in UI onto key of programsListModel.params hash	
	var paramName = mapParamName(paramNameSrc);
	
	// obtain the parameter definition object
	var curParamDef=programsListModel.params[paramName];
	if (curParamDef==null) return false;
	if (!curParamDef["isValid"]) return false;
	
	// check for type=='register' and index[progIdx]==REG_IDX_NA
	var type  = curParamDef.type[progIdx];
	var regIndex = curParamDef.index[progIdx];
	if (type==PARAM_TYPE_REGISTER && regIndex==REG_IDX_NA) return false;

	// reserved for future use:
	if (paramNameSrc=="band3") return false;
	
	// we assume active parameter by default
	return true;
}

//================ restore parameter values from cppModel ====================
function resetParam(progIdx,paramNameSrc) {
	
    // Map parameter name in UI onto key of programsListModel.params hash	
	var paramName = mapParamName(paramNameSrc);
	
	// retrieve the parameter definition
	var curParamDef=programsListModel.params[paramName];
	if (curParamDef==null) {
		// cppModel.msg("In resetParam: curParamDef==null: "+paramName);
		return;
	}
	
	var mask  = curParamDef.mask[progIdx];
	var shift = curParamDef.shift[progIdx];
	var type  = curParamDef.type[progIdx];
	var index = curParamDef.index[progIdx];
	var isScalar = curParamDef.isScalar;
	var value    = null;
	if (type==PARAM_TYPE_REGISTER)
		value = cppModel.getRegisterValue(index, mask, shift);
	if (type==PARAM_TYPE_FREQTBL)
		value = cppModel.getFrequencyValue(index, mask, shift);
	// cppModel.msg("In reset param: "+paramNameSrc+" progIdx: "+progIdx+"  value:"+value);
	if (paramNameSrc == "freqFromHB") {
		value &= 0x0000FFFF;
		value *= 10;
	}
	if (paramNameSrc == "freqFromLB") {
		value >>= 16;
		value &= 0x0000FFFF;
	}
	if (paramNameSrc == "freqToHB") {
		value &= 0x0000FFFF;
		value *= 10;
	}
	if (paramNameSrc == "freqToLB") {
		value >>= 16;
		value &= 0x0000FFFF;
	}
	if (paramNameSrc=="allSectors") return;
	if (/^sector[0-7]$/.exec(paramNameSrc)) {
        var bit=parseInt(paramNameSrc.charAt(6));
		value >>= bit;
		value &= 0x00000001;
	}
	if (paramNameSrc=="allAttenuators") return;
	if (/^attenuator[0-7]$/.exec(paramNameSrc)) {
        var bit=parseInt(paramNameSrc.charAt(10));
		value >>= bit;
		value &= 0x00000001;
	}
	// decode frequency band
	if (paramName == "band") {
		value = 
		    (value==0x00)&&(paramNameSrc=="band40")
		 || (value==0x01)&&(paramNameSrc=="band20")
		 || (value==0x02)&&(paramNameSrc=="band10")
		 // reserved for future use: 
		 // || (value==0x04)&&(paramNameSrc=="band3");
		    ;
    }
	// reset strob duration
	if (paramName == "analysis") {
        switch (value) {
			case 0x00: value=  5; break;
			case 0x01: value= 10; break;
			case 0x02: value= 50; break;
			case 0x04: value=200; break;
			default: value=5;
		}
	}	
	// update programsListModel
	setParam(progIdx,paramNameSrc, value);
}

//================ restore all parameter values from cppModel ====================
function resetAllParams(progIdx) {
    // if (progIdx==0) cppModel.msg("In resetAllParams for Idx0");
	var obj = programsListModel.get(progIdx);
	for (var paramNameSrc in obj) {
		//== optimization: calling resetParam is not effective
		// resetParam(progIdx,paramNameSrc);
		
		//== skip unused keys of ListElement
		if (paramNameSrc=="name" || paramNameSrc=="enabled") continue;
		if (!isParamEnabled(progIdx,paramNameSrc)) continue;
		// Map parameter name in UI onto key of programsListModel.params hash
		var paramName = mapParamName(paramNameSrc);
		//== obtain the parameter definition object 
		var curParamDef=programsListModel.params[paramName];
		if (curParamDef==null) {
			cppModel.msg("not a key of programsListModel.params "+paramNameSrc+" -> "+paramName);
			continue; // parameter is not a key of programsListModel.params hash
		}
		
		var mask  = curParamDef.mask[progIdx];
		var shift = curParamDef.shift[progIdx];
		var type  = curParamDef.type[progIdx];
		var index = curParamDef.index[progIdx];
		var isScalar = curParamDef.isScalar;
		var value    = null;
		if (type==PARAM_TYPE_REGISTER)
			value = cppModel.getRegisterValue(index, mask, shift);
		if (type==PARAM_TYPE_FREQTBL)
			value = cppModel.getFrequencyValue(index, mask, shift);
		//== perform necessary transformations for frequencies
		if (paramNameSrc == "freqFromHB") {
			value &= 0x0000FFFF;
			value *= 10;
		}
		if (paramNameSrc == "freqFromLB") {
			value >>= 16;
			value &= 0x0000FFFF;
		}
		if (paramNameSrc == "freqToHB") {
			value &= 0x0000FFFF;
			value *= 10;
		}
		if (paramNameSrc == "freqToLB") {
			value >>= 16;
			value &= 0x0000FFFF;
		}
		//== process 'sectors'
		if (~paramNameSrc.indexOf("sector")) continue;
		if (paramNameSrc=='allSectors') {
			var bits=[];
			var allSectors=true;
			for (var i=0; i<8; i++) {
                bits[i]=!!((+value>>i)&0x00000001);
				if (!bits[i]) allSectors=false;
			}
			programsListModel.set(progIdx,{
				sector0:bits[0],sector1:bits[1],sector2:bits[2],sector3:bits[3],
				sector4:bits[4],sector5:bits[5],sector6:bits[6],sector7:bits[7],
				allSectors:allSectors
			});
			continue;
		}
		//== process 'attenuators'
		if (~paramNameSrc.indexOf("attenuator")) continue;
		if (paramNameSrc=='allAttenuators') {
			var bits=[];
			var allAttenuators=1;
			for (var i=0; i<8; i++) {
                bits[i]=(value>>i)&0x00000001;
				if (!bits[i]) allAttenuators=0;
			}
			programsListModel.set(progIdx,{
				attenuator0:bits[0],attenuator1:bits[1],attenuator2:bits[2],attenuator3:bits[3],
				attenuator4:bits[4],attenuator5:bits[5],attenuator6:bits[6],attenuator7:bits[7],
				allAttenuators:allAttenuators
			});
			continue;
		}
		// decode frequency band
		if (paramName == "band") {
			value = 
				(value==0x00)&&(paramNameSrc=="band40")
			 || (value==0x01)&&(paramNameSrc=="band20")
			 || (value==0x02)&&(paramNameSrc=="band10")
			 // reserved for future use: 
			 // || (value==0x04)&&(paramNameSrc=="band3");
				;
		}
		// reset strob duration
		if (paramName == "analysis") {
			switch (value) {
				case 0x00: value=  5; break;
				case 0x01: value= 10; break;
				case 0x02: value= 50; break;
				case 0x04: value=200; break;
				default: value=5;
			}
		}	
		//== by default just set property value
		programsListModel.setProperty(progIdx,paramNameSrc,value);
	}
}

//================= This function is called on cppModel signal paramsChanged =================
function resetAllParamsAllSubprograms() {
    // cppModel.msg("resetAllParamsAllSubprograms(): "+JSON.stringify(programsListModel.params));
	// get currentModesMask
	var programsCount=programsListModel.count;
	var currentModesMaskDef=programsListModel.params["current_modes_mask"];
	if (currentModesMaskDef != null) {
		var arrMask=currentModesMaskDef["mask"];
		var arrShift=currentModesMaskDef["shift"];
		var arrType=currentModesMaskDef["type"];
		var arrIndex=currentModesMaskDef["index"];
		var isValid=currentModesMaskDef["isValid"];
		var isScalar=currentModesMaskDef["isScalar"];
		if (!isScalar) isValid=false;
		if (programsCount>32) isValid=false;
	}
	var currentModesMask=null;
	if (currentModesMaskDef != null && isValid) {
		var mask=arrMask[0];
		var shift=arrShift[0];
		var type=arrType[0];
		var index=arrIndex[0];
		var value=null;
		if (type==PARAM_TYPE_REGISTER)
			currentModesMask = cppModel.getRegisterValue(index, mask, shift);
		if (type==PARAM_TYPE_FREQTBL)
			currentModesMask = cppModel.getFrequencyValue(index, mask, shift);
		// cppModel.msg("currentModesMask="+currentModesMask);
	}
	// loop over subprograms
	for (var progIdx=0; progIdx<programsCount; progIdx++) {
		// call optimized reset subroutine
		resetAllParams(progIdx);
		// reset subprogram enabled status
		if (currentModesMask != null) {
			var bEnabled = !!((1<<progIdx)&currentModesMask);
			setEnabled(progIdx,bEnabled);
		}
	}
}

//================ get current (possibly modified) parameter value from QML model "programsListModel" ============
function getParam(progIdx,paramName) {
	var retval=programsListModel.get(progIdx)[paramName];
	if (retval!=null) 
		return retval;
	else 
		return 0;
}

//================ enable/disable subprograms ======================================
function setEnabled(progIdx,bEnabled) {
	programsListModel.set(progIdx,{"enabled": bEnabled});
}

//================ get subprogram "enabled" value ==================================
function getEnabled(progIdx) {
	return programsListModel.get(progIdx)["enabled"];
}

//================ get subprogram index for the visible tabWidget tab ==============
function getCurrentProgramIndex() {
	return tabWidget.current;
}

//================ get subprogram name =============================================
function getName(progIdx) {
	return programsListModel.get(progIdx)["name"];
}

//================ set parameter value in QML model "programsListModel" ============
function setParam(progIdx,paramNameSrc,val) {
	// validate frequency
        // cppModel.msg("In setParam: progIdx="+progIdx+" paramNameSrc="+paramNameSrc+" val="+val );	
	if (paramNameSrc == "freqFromHB" || paramNameSrc == "freqToHB") {
		val=Math.round(parseFloat(val)/10.0)*10; // must be multiple of 10 MHz
	}
	
    // Map parameter name in UI onto key of programsListModel.params hash	
	var paramName = mapParamName(paramNameSrc);
	// obtain parameter definition object
	var curParamDef=programsListModel.params[paramName];
	if (curParamDef==null) return false; // not a key of programsListModel.params hash

	var isScalar = curParamDef.isScalar;
	
	for (var i=0; i<programsListModel.count; i++) {
		if ( isScalar || i==progIdx) {
			
			if (paramNameSrc=='allSectors') programsListModel.set(i,{
				sector0:val,sector1:val,sector2:val,sector3:val,
				sector4:val,sector5:val,sector6:val,sector7:val
			});
			if (paramNameSrc=='allAttenuators') programsListModel.set(i,{
				attenuator0:val,attenuator1:val,attenuator2:val,attenuator3:val,
				attenuator4:val,attenuator5:val,attenuator6:val,attenuator7:val
			});
			
			if (paramName == "band") { // band switch: 3 10 20 40 MHz
				// cppModel.msg("changing band. paramNameSrc="+paramNameSrc+" i="+i+" val="+val+" +true="+(+true));
				if (val) { // only setting bandXX=1 is informative!
					// cppModel.msg("changing band");
					programsListModel.set(i,{
						band3:  +(paramNameSrc=='band3'),  band10: +(paramNameSrc=='band10'),
						band20: +(paramNameSrc=='band20'), band40: +(paramNameSrc=='band40')
					});
				}
				// cppModel.msg("continuing with value: " + programsListModel.get(i)[paramNameSrc]);
				continue;
			}
			var obj = {}; obj[paramNameSrc]=val;
			// cppModel.msg("setting paramNameSrc="+paramNameSrc+" val="+val);
			programsListModel.set(i,obj);
		}
	}
	return true;
}

//========== this function defines the set of subprograms and the corresponding parameters ============
function initProgramsListModel() {
	var objParamsVal={
		"sector0":0,"sector1":0,"sector2":0,"sector3":0,"sector4":0,"sector5":0,
		"sector6":0,"sector7":0,"allSectors":0,"attenuator0":0,
		"attenuator1":0,"attenuator2":0,"attenuator3":0,"attenuator4":0,"attenuator5":0,
		"attenuator6":0,"attenuator7":0,"allAttenuators":0,"freqFromHB":3000,"freqToHB":3030,
		"freqFromLB":200,"freqToLB":300,"band3":0,"band10":0,"band20":0,"band40":1,		
		"peleng":1,"calibr":1,"analysis":10,"nstrobs":100,"enabled":true,
		"scan":0
	}
	objParamsVal["name"]='PRG1';
	programsListModel.append(objParamsVal);
	objParamsVal["name"]='PRG2';
	programsListModel.append(objParamsVal);
	objParamsVal["name"]='CP1';
	programsListModel.append(objParamsVal);
	objParamsVal["name"]='GP10';
	programsListModel.append(objParamsVal);
	objParamsVal["name"]='AR15';
	programsListModel.append(objParamsVal);
	objParamsVal["name"]='TA0';
	programsListModel.append(objParamsVal);
	objParamsVal["name"]='СПЕЦ';
	programsListModel.append(objParamsVal);
	objParamsVal["name"]='ЦЕЛЬ';
	programsListModel.append(objParamsVal);
	forceActiveFocus();
}

