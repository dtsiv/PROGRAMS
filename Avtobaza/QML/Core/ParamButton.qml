import QtQuick 1.0
import "codograms.js" as CodogramsJS

Button {
    id: paramButton
    property string paramName: ""
    property int programIndex: 0
    property bool enabled: CodogramsJS.getParam(paramButton.programIndex,paramButton.paramName); 
	visible: CodogramsJS.isParamEnabled(paramButton.programIndex,paramButton.paramName)
 
    onClicked: {
		CodogramsJS.setParam(paramButton.programIndex,
                        paramButton.paramName,
                       !paramButton.enabled);
	}

    state: paramButton.enabled?"checked":"";
}
