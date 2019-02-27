/*================================================================================
 *  functions used to parse LOCALCTRLSTRUCTEX_::mode[iSelected].sConventions
 *  and to perform local bookkeeping, including masks, shift amounts, 
 *  register and frequency table indices
 *================================================================================*/
 
// this is not stateless library 
// .pragma library

Qt.include("paramio.js")

// not more than 140 freqtbl entries
var FREQUTBL_MAX_IDX=140;
// currently, register indices 0..7=A,B,C,D,E,F,Fh,CND; index 8 means Not Assigned (NA)
var arrRegisters = ["A","B","C","D","E","F","Fh","CND","NA"];
var REG_IDX_NA=8;
var allOnes=0xFFFFFFFF;
var PARAM_TYPE_REGISTER='register';
var PARAM_TYPE_FREQTBL='freqtbl';

//=============== parse the conventions string ==========================
function parseConventions() {
	// objParamsDef - resulting object with parameters definitions
	// keys - parameter names like sector0,sector1,...,peleng etc
	// values - objects with parameter definitions: {
	//     index[], // index of register (=0,...,7); index of freqtbl (=0,...,139)
	//     mask[],  // quint32 - mask used with target register. By default 0xFFFFFFFF or 11...1
	//     shift[], // quint32 - shift corresponding to parameter position in register, i.e. 
	//                         lowest non-zero bit in the mask
	//     type     // 'register' or 'freqtbl'
	// }    
	// index[], mask[], shift[] - are all arrays with index subProgramId (=0,1,...,7)
	
	var programsCount=programsListModel.count;
	var objParamsDef={};
	// conventions string
	var sConventions=cppModel.conventions;
	// individual parameters definitions
	var arrParamDefs=sConventions.split(';');
	
	// cppModel.msg("arrParamDefs.length="+arrParamDefs.length);
	
	// loop over parameters. Check if this is array or scalar
	var i;
	for (i=0; i<arrParamDefs.length; i++) {
		var paramDef = arrParamDefs[i].split('=');
		if (paramDef.length!=2) continue; // parameter must be defined as paramName=paramDef
		var paramName = paramDef[0];
		paramDef = paramDef[1];
		//======= match modes mask =======
		if (paramName == 'modes_mask') {
			for (var ib=0; ib<Math.min(programsCount,paramDef.length); ib++) {
				var enabled=parseInt(paramDef.substr(ib,1));
				programsListModel.setProperty(ib,"enabled",!!enabled);
			}
			continue;
		}
		//========= match array ==========
		var re = /^\[(.*)\]$/i;
		var bBracketNotation;
		var match=re.exec(paramDef);
		if (match && match.length==2) {
			bBracketNotation=true;
			paramDef=match[1];
		}
		else
			bBracketNotation=false;
		// OK, now we have paramDef as space-separated list of parameter definition substrings,
		// one substring per subprogram. Start actual parsing
		var curParamDef = {
			"paramDef":         paramDef,
			"bBracketNotation":  bBracketNotation
		};
		// run parsing
		parseParamDef(curParamDef,programsCount);
		// save results in the output array
		objParamsDef[paramName] = curParamDef;
		// console.log(JSON.stringify(curParamDef));
	}
	
	var typeFreqTbl =[];
	var typeRegister=[];
	var shiftZero=[];
	var maskAllOnes=[];
	
	var progIdx;
	for (progIdx=0; progIdx<programsCount; progIdx++) {
		typeFreqTbl[progIdx]  = PARAM_TYPE_FREQTBL;
		typeRegister[progIdx] = PARAM_TYPE_REGISTER;
		shiftZero[progIdx]    = 0;
		maskAllOnes[progIdx]  = allOnes;
	}
	
	// here go parameters not listed in sConventions. 
	// Currently, only use listed parameters
	var arrStoreInRegisters = [];
	var arrStoreInFreqTbl   = [];
/*	
	var arrStoreInFreqTbl   = [
            "freqFrom","freqTo",
            "sector0","sector1","sector2","sector3","sector4","sector5","sector6","sector7","allSectors", // Freq2-10
            "attenuator0","attenuator1","attenuator2","attenuator3","attenuator4","attenuator5","attenuator6","attenuator7","allAttenuators", // Freq11-19
            "analysis","nstrobs","peleng","calibr",
            "band3","band10","band20","band40"];
*/			
	var arrayOfEqualIndices=[];
	var regIdx;
	for (regIdx=0; regIdx<Math.max(arrStoreInRegisters.length,arrStoreInFreqTbl.length); regIdx++) {
		arrayOfEqualIndices[regIdx]=[];
		for (progIdx=0; progIdx<programsCount; progIdx++) {
			arrayOfEqualIndices[regIdx][progIdx]=regIdx;
		}
	}
	for (regIdx=0; regIdx<Math.min(arrStoreInRegisters.length,REG_IDX_NA); regIdx++) { // not more than 8 registers
		if(!(arrStoreInRegisters[regIdx] in objParamsDef)) objParamsDef[arrStoreInRegisters[regIdx]]={
			index:    arrayOfEqualIndices[regIdx], 
			type:     typeRegister,
			isScalar: true, 
			mask:     maskAllOnes,
			shift:    shiftZero			
		};
	}
	for (regIdx=REG_IDX_NA; regIdx<arrStoreInRegisters.length; regIdx++) { // disable extra parameters
		if(!(arrStoreInRegisters[regIdx] in objParamsDef)) 
			objParamsDef[arrStoreInRegisters[regIdx]]={
				index:    arrayOfEqualIndices[REG_IDX_NA], 
				type:     typeRegister,
				isScalar: true, 
				mask:     maskAllOnes,
				shift:    shiftZero			
			};
	}
	for (regIdx=0; regIdx<Math.min(arrStoreInFreqTbl.length,FREQUTBL_MAX_IDX); regIdx++) {  
		if(!(arrStoreInFreqTbl[regIdx] in objParamsDef)) objParamsDef[arrStoreInFreqTbl[regIdx]]={
			index:    arrayOfEqualIndices[regIdx], 
			type:     typeFreqTbl,
			isScalar: true, 
			mask:     maskAllOnes,
			shift:    shiftZero			
		};
	}
	
    // assign the resulting object 	
	programsListModel.params = objParamsDef;
	
	// cppModel.msg((JSON.stringify(objParamsDef)).substr(0*498,498));
	// cppModel.msg((JSON.stringify(objParamsDef)).substr(1*498,498));
	// cppModel.msg((JSON.stringify(objParamsDef)).substr(2*498,498));
	// cppModel.msg((JSON.stringify(objParamsDef)).substr(3*498,498));
	
	return;
}

//============ parse the individual parameter definition substring =================
function parseParamDef(objParamDef,programsCount) {
	var re;
	var match;
	var i, shift, mask, range, ftStart, ftEnd;
	var paramDef=objParamDef.paramDef;
    // initialize the return object objParamDef:
	// { "type": [register freqtbl freqtbl register ...],
	//   "mask": [0xFFFFFFFF 0x01 0x10 0x11 ...],
	//   "shift": [0 0 4 0 ...],
	//   "index": [0 1 2 3 ...],
	//   "isValid": [true true true true ...],
	// }
	//
	var arrShifts = [];
	objParamDef["shift"] = arrShifts;
	var arrMasks = [];
	objParamDef["mask"] = arrMasks;
	var arrTypes = [];
	objParamDef["type"] = arrTypes;
	var arrIndices = [];
	objParamDef["index"] = arrIndices;
	//== by default assume vector parameter. 
	//== Scalar if: no [] brackets and single lexem and not FT(start:end) notation
	objParamDef["isScalar"] = false;
	
	//== parameter was defined using [] brackets?
	var bBracketNotation=objParamDef["bBracketNotation"];
	
	// split into vector elements over space separator
	re = /\s+/;
	var lexems = paramDef.split(re);
	// return object is not valid by default
	objParamDef["isValid"] = false;
	if (lexems.length != programsCount && lexems.length != 1) return;  // invalid definition for parameter
	objParamDef["isValid"] = true;
	// loop over elements substrings
	for (var il=0; il<lexems.length; il++) {
		var lexem = lexems[il];
		//========= match whole register
		re = /^\s*(A|B|C|D|E|F|Fh|CND|NA)\s*$/;
		match=re.exec(lexem);
		if (match && match.length==2) {
			arrTypes.push(PARAM_TYPE_REGISTER);
			arrIndices.push(arrRegisters.indexOf(match[1]));
			arrShifts.push(0);
			arrMasks.push(allOnes);
			//========= 
			if (lexems.length==1 && !bBracketNotation) objParamDef["isScalar"] = true;
			continue;
		}
		//========= match register with mask: A(4,5,6,7)
		re = /^\s*(A|B|C|D|E|F|Fh|CND)\(([0-9,]+)\)\s*$/;
		match=re.exec(lexem);
		if (match && match.length==3) {
			arrTypes.push(PARAM_TYPE_REGISTER);
			arrIndices.push(arrRegisters.indexOf(match[1]));
			var bits=(match[2]).split(',');
			//== shift is zero-based number of lowest bit
			shift=32; mask=0;
			for (i=0; i<bits.length; i++) {
				shift=Math.min(shift,bits[i]);
				mask |= (1<<bits[i]);
			}
			arrShifts.push(shift);
			arrMasks.push(mask);
			if (lexems.length==1 && !bBracketNotation) objParamDef["isScalar"] = true;
			continue;
		}
		//========= match register with mask: A(4:7)
		re = /^\s*(A|B|C|D|E|F|Fh|CND)\(([0-9]+):([0-9]+)\)\s*$/;
		match=re.exec(lexem);
		if (match && match.length==4 && +match[2]<+match[3]) {
			arrTypes.push(PARAM_TYPE_REGISTER);
			arrIndices.push(arrRegisters.indexOf(match[1]));
			//== shift is zero-based number of lowest bit
			shift=+match[2]; mask=0;
			for (i=+match[2]; i<=+match[3]; i++) {
				mask |= (1<<i);
			}
			arrShifts.push(shift);
			arrMasks.push(mask);
			if (lexems.length==1 && !bBracketNotation) objParamDef["isScalar"] = true;
			continue;
		}
        //========= match freq
		re = /^\s*FT\((.*)\)\s*$/;
		match=re.exec(lexem);
		if (match && match.length==2) {
			var ftStart, ftEnd;
			//== parse range definition
			range=(match[1]).split(':');        // FT(ftStart:ftEnd) 
			if ((bBracketNotation && range.length!=1) || range.length>2 || range.length<1) {
				objParamDef["isValid"] = false; // parameter can be either defined with FT(ftStart:ftEnd) 
				return;                         // or with [A B FT(33) C FT(32)]. Cannot use nested vectors!
			}
			ftStart=parseInt(range[0]);
			if (range.length==1) {              // one frequency table cell
				arrTypes.push(PARAM_TYPE_FREQTBL);
				arrIndices.push(ftStart);
				arrShifts.push(0);
				arrMasks.push(allOnes);
				if (lexems.length==1 && !bBracketNotation) objParamDef["isScalar"] = true;
				continue;
			}
			// array FT(ftStart:ftEnd) with programsCount elements
			ftStart=parseInt(range[0]);
			ftEnd=parseInt(range[1]);
			if (ftEnd-ftStart != programsCount-1  // array FT(ftStart:ftEnd) elements count mismatch.
			            || il >  0                // previous array elements exist. Cannot mix array forms
			    ) {
				objParamDef["isValid"] = false; // number of elements in FT(ftStart:ftEnd) must match programsCount
				return;
			}
			
			for (i=0; i<programsCount; i++) {
				arrTypes.push(PARAM_TYPE_FREQTBL);
				arrIndices.push(ftStart+i);
				arrShifts.push(0);
				arrMasks.push(allOnes);
			}
			// console.log("FT vector: "+JSON.stringify(objParamDef));
		}
		// no match: syntax error in paramDef
		objParamDef["isValid"] = false;
		return;
	}
	// if this is scalar parameter, make it vector with length programsCount
	if (objParamDef["isScalar"]==true) {
		for (i=1; i<programsCount; i++) {
			arrTypes.push(arrTypes[0]);
			arrIndices.push(arrIndices[0]);
			arrShifts.push(arrShifts[0]);
			arrMasks.push(arrMasks[0]);
		}
	}
	// successfully finished parsing this parameter's paramDef string
	return;
}
