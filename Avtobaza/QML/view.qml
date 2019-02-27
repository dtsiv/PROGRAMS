//=================================================================================================
// QML sources for Avtobaza-MR plugin QLocalControlQMLPlugin Version 2.2
// Setup: Qt 4.7.2 built from source, QtQuick 1.0, qtcreator 2.8.1 binary installer
//=================================================================================================
import QtQuick 1.0

//======= all necessary small components are located in the Core directory =======
import "Core"
import "Core/codograms.js" as CodogramsJS

//================= use FocusScope for the root element in order to accept hotkeys ===============
FocusScope {
    anchors.fill: parent

	//=============== check is shortcut key was pressed =====================
    Keys.onPressed: {
	var i = cppModel.matchShortCut(event.key,event.modifiers);
	if (i>=0) tabWidget.current = i;
    }

	//================ cppModel is created from C++, use Connections section to access signals ===========
    Connections {
        target: cppModel
        onConventionsChanged: {
            CodogramsJS.parseConventions();
        }
        onParamsChanged: {
            CodogramsJS.resetAllParamsAllSubprograms();
            cppModel.refreshControls();
        }
        onTargetSelected: {
            CodogramsJS.setParam(7,"freqFromHB",freqFromHB);
            CodogramsJS.setParam(7,"freqToHB",freqToHB);
            cppModel.refreshControls();
        }
    }

	//================ populate QML model "programsListModel" at startup ============
    Component.onCompleted: {
		CodogramsJS.initProgramsListModel();
    }

    //===============  local model - used for QML =========================
    ListModel {
        id: programsListModel
		property variant params: []
    }
 
    //===============  left column - buttons panel  =======================
    Rectangle {
        id: buttonsPanel
        color: "#343434"
        width: parent.width * 0.2
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
        // visible: false
        visible: true

        Column {
            id: buttonsColumn
            spacing: 6; anchors.margins: buttonsColumn.spacing;
            anchors.fill: parent
            property real h: (buttonsPanel.height - buttonsColumn.spacing) / programsListModel.count
                           - buttonsColumn.spacing;
            property real w: buttonsPanel.width - 2*buttonsColumn.spacing;

            Repeater {
                id: buttonsRepeater
                model: programsListModel
                delegate: ProgramSwitch {
                    id: programSwitch; programIndex: model.index; programName: model.name;
                    width: buttonsColumn.w; height: buttonsColumn.h;
                    onCheckButtonClicked: tabWidget.current = model.index
                    onButtonClicked: tabWidget.current = model.index
                }
            }
        }
    }

	//========= this TabWidget is taken from QML demos. Modified here for our purpose ==================
    TabWidget {
		id: tabWidget
//		visible: false
		visible: true
		anchors { left: buttonsPanel.right; top: parent.top; bottom: parent.bottom; right: parent.right;}
		model: programsListModel
		signal userInput(int iTab)
		onUserInput: console.log("User clicked on button at tab #" + iTab)
		  
		delegate: ControlPanelDelegate {
		    programIndex: model.index
		}
    }

/*	
 *	PasswordDialog {
 *          anchors { left: parent.left; top: parent.top; bottom: parent.bottom; right: parent.right }
 *	    id: passwdDialog
 *	}
 */
}
