import QtQuick 1.0
import "codograms.js" as CodogramsJS

SearchBox {
    id: paramInput
    property string paramName: ""
    property int programIndex: 0
    text: CodogramsJS.getParam(paramInput.programIndex,paramInput.paramName)
    visible: CodogramsJS.isParamEnabled(paramInput.programIndex,paramInput.paramName)
	
    onActiveFocusChanged: { 
        if (!paramInput.activeFocus) {
			var newVal=paramInput.text;
			CodogramsJS.setParam(paramInput.programIndex,
								paramInput.paramName,
								paramInput.text);
			// refresh controls on invalid input
			if (newVal!=paramInput.text) cppModel.refreshControls();
        }
    }                  
}

