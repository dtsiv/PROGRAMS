import QtQuick 1.0
import "codograms.js" as CodogramsJS

Item {
    id: programSwitch
    property int programIndex: 0
    property string programName: "PRG"

    signal buttonClicked
    signal checkButtonClicked

    onCheckButtonClicked: {
		CodogramsJS.setEnabled(programIndex,!CodogramsJS.getEnabled(programIndex));
    }

    Row {
        id: modeRow; spacing: 6; width: parent.width; height: parent.height;

        Button {
            id: programEnableButton
            width: (parent.width - parent.spacing) * 0.3
            height: parent.height
            color: 'red'
            operation: ""
			pressbutton: true
            state: (CodogramsJS.getEnabled(programIndex))?"":"checked"
            onClicked: {
                programSwitch.checkButtonClicked();
                programEnableButton.forceActiveFocus()
            }
        }

        Button {
            id: programButton
            width: (parent.width - parent.spacing) * 0.7
            height: parent.height
            color: 'green'
            operation: programSwitch.programName
			pressbutton: true
            state: (CodogramsJS.getCurrentProgramIndex()==programIndex)?"":"checked"
            onClicked: {
                programSwitch.buttonClicked();
                programButton.forceActiveFocus()
            }
            onRightClicked: {
                if (operation=="ЦЕЛЬ") cppModel.selectTarget();
	    }
        }
    }
}


