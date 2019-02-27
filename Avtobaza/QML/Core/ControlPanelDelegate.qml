import QtQuick 1.0
import "codograms.js" as CodogramsJS

Rectangle {
    id: tab

    property int programIndex: 0
    property string title: CodogramsJS.getName(programIndex)
	property bool isEditable: title!="ЦЕЛЬ"
    anchors.fill: parent
    color: "#ffffff"

    Item {
        id: iGr
        property real  gx: parent.width / 42
        property real  gy: parent.height / 42
        property real   w: iGr.gx * 4
        property real   h: iGr.gy * 4
        property real gxc: parent.width / 2
        property real gyc: iGr.gy * 7 + iGr.h * 6
    }

    ParamLabel {
        x: iGr.gx; y: iGr.gy;
        width: iGr.gx * 2 + iGr.w * 3; height: iGr.h;
        text: "Секторы";
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"allSectors")
    }

    ParamButton {
        x: iGr.gx * 4 +  iGr.w * 3; y: iGr.gy;
        width: iGr.w; height: iGr.h;
        paramName: "allSectors"
        operation: "Все"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx; y: iGr.gy * 2 + iGr.h;
        width: iGr.w; height: iGr.h;
        paramName: "sector0"
        operation: "0"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 2 +  iGr.w; y: iGr.gy * 2 + iGr.h;
        width: iGr.w; height: iGr.h;
        paramName: "sector1"
        operation: "1"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 3 +  iGr.w * 2; y: iGr.gy * 2 + iGr.h;
        width: iGr.w; height: iGr.h;
        paramName: "sector2"
        operation: "2"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 4 +  iGr.w * 3; y: iGr.gy * 2 + iGr.h;
        width: iGr.w; height: iGr.h;
        paramName: "sector3"
        operation: "3"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx; y: iGr.gy * 3 + iGr.h * 2;
        width: iGr.w; height: iGr.h;
        paramName: "sector4"
        operation: "4"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 2 +  iGr.w; y: iGr.gy * 3 + iGr.h * 2;
        width: iGr.w; height: iGr.h;
        paramName: "sector5"
        operation: "5"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 3 +  iGr.w * 2; y: iGr.gy * 3 + iGr.h * 2;
        width: iGr.w; height: iGr.h;
        paramName: "sector6"
        operation: "6"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 4 +  iGr.w * 3; y: iGr.gy * 3 + iGr.h * 2;
        width: iGr.w; height: iGr.h;
        paramName: "sector7"
        operation: "7"
        programIndex: tab.programIndex
    }

    ParamLabel {
        x: iGr.gx; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy;
        width: iGr.w * 3 + iGr.gx * 2; height: iGr.h;
        text: "Подавление";
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"allAttenuators")
    }

    ParamButton {
        x: iGr.gx * 4 +  iGr.w * 3; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy;
        width: iGr.w; height: iGr.h;
        paramName: "allAttenuators"
        operation: "Все"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy * 2 + iGr.h;
        width: iGr.w; height: iGr.h;
        paramName: "attenuator0"
        operation: "0"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 2 +  iGr.w; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy * 2 + iGr.h;
        width: iGr.w; height: iGr.h;
        paramName: "attenuator1"
        operation: "1"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 3 +  iGr.w * 2; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy * 2 + iGr.h;
        width: iGr.w; height: iGr.h;
        paramName: "attenuator2"
        operation: "2"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 4 +  iGr.w * 3; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy * 2 + iGr.h;
        width: iGr.w; height: iGr.h;
        paramName: "attenuator3"
        operation: "3"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy * 3 + iGr.h * 2;
        width: iGr.w; height: iGr.h;
        paramName: "attenuator4"
        operation: "4"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 2 +  iGr.w; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy * 3 + iGr.h * 2;
        width: iGr.w; height: iGr.h;
        paramName: "attenuator5"
        operation: "5"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 3 +  iGr.w * 2; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy * 3 + iGr.h * 2;
        width: iGr.w; height: iGr.h;
        paramName: "attenuator6"
        operation: "6"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 4 +  iGr.w * 3; y: iGr.gy * 3 + iGr.h * 3 + iGr.gy * 3 + iGr.h * 2;
        width: iGr.w; height: iGr.h;
        paramName: "attenuator7"
        operation: "7"
        programIndex: tab.programIndex
    }

    ParamLabel {
        x: iGr.gxc + iGr.gx; y: iGr.gy;
        width: iGr.gx * 3 + iGr.w * 4; height: iGr.h;
        text: "Диапазон ВЧ (МГц)";
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"freqFromHB")
    }

    ParamInput {
        x: iGr.gxc + iGr.gx; y: iGr.gy * 2 + iGr.h;
        width: iGr.w * 2; height: iGr.h;
        paramName: "freqFromHB"
        programIndex: tab.programIndex
		textInputIsEditable: tab.isEditable
    }

    Text {
        x: iGr.gxc + iGr.gx * 2 + iGr.w * 2; y: iGr.gy * 2 + iGr.h;
        width: iGr.gx; height: iGr.h;
        text: "-"; font.pixelSize: iGr.h;
		verticalAlignment: Text.AlignVCenter;
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"freqToHB")
    }

    ParamInput {
        x: iGr.gxc + iGr.gx * 4 + iGr.w * 2; y: iGr.gy * 2 + iGr.h;
        width: iGr.w * 2; height: iGr.h;
        paramName: "freqToHB"
        programIndex: tab.programIndex
		textInputIsEditable: tab.isEditable
    }

    ParamLabel {
        x: iGr.gxc + iGr.gx; y: iGr.gy * 3 +  iGr.h * 2;
        width: iGr.gx * 3 + iGr.w * 4; height: iGr.h;
        text: "Диапазон НЧ (МГц)";
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"freqFromLB")
    }

    ParamInput {
        x: iGr.gxc + iGr.gx; y: iGr.gy * 4 + iGr.h * 3;
        width: iGr.w * 2; height: iGr.h;
        paramName: "freqFromLB"
        programIndex: tab.programIndex
    }

    Text {
        x: iGr.gxc + iGr.gx * 2 + iGr.w * 2; y: iGr.gy * 4 + iGr.h * 3;
        width: iGr.gx; height: iGr.h;
        text: "-"; font.pixelSize: iGr.h;
		verticalAlignment: Text.AlignVCenter;
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"freqToLB")
    }

    ParamInput {
        x: iGr.gxc + iGr.gx * 4 + iGr.w * 2; y: iGr.gy * 4 + iGr.h * 3;
        width: iGr.w * 2; height: iGr.h;
        paramName: "freqToLB"
        programIndex: tab.programIndex
    }

    ParamLabel {
        x: iGr.gxc + iGr.gx * 2 + iGr.w; y: iGr.gy * 5 +  iGr.h * 4;
        width: iGr.gx * 2 + iGr.w * 3; height: iGr.h;
        text: "Полоса частот (МГц)";
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"band40")
    }

    ParamButton {
        x: iGr.gxc + iGr.gx; y: iGr.gy * 6 + iGr.h * 5;
        width: iGr.w; height: iGr.h;
        paramName: "band3"
        operation: "3"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gxc + iGr.gx * 2 + iGr.w; y: iGr.gy * 6 + iGr.h * 5;
        width: iGr.w; height: iGr.h;
        paramName: "band10"
        operation: "10"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gxc + iGr.gx * 3 + iGr.w * 2; y: iGr.gy * 6 + iGr.h * 5;
        width: iGr.w; height: iGr.h;
        paramName: "band20"
        operation: "20"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gxc + iGr.gx * 4 + iGr.w * 3; y: iGr.gy * 6 + iGr.h * 5;
        width: iGr.w; height: iGr.h;
        paramName: "band40"
        operation: "40"
        programIndex: tab.programIndex
    }

    ParamLabel {
        x: iGr.gx * 1 + iGr.w * 0; y: iGr.gyc + iGr.gy * 1 + iGr.h * 0;
        width: iGr.gx * 3 + iGr.w * 2; height: iGr.h;
        text: "Тип обзора";
        hAlign: Text.AlignLeft
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"scan")
    }
	
    ParamInput {
        x: iGr.gx * 5 + iGr.w * 1; y: iGr.gyc + iGr.gy * 1 + iGr.h * 0;
        width: iGr.gx * 0 + iGr.w * 1; height: iGr.h;
        paramName: "scan"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 1 + iGr.w * 0; y: iGr.gyc + iGr.gy * 1 + iGr.h * 0;
        width: iGr.w * 3 + iGr.gx * 2; height: iGr.h;
        paramName: "peleng"
        operation: "Пеленг"
        programIndex: tab.programIndex
    }

    ParamButton {
        x: iGr.gx * 1 + iGr.w * 0; y: iGr.gyc + iGr.gy * 2 + iGr.h * 1;
        width: iGr.w * 3 + iGr.gx * 2; height: iGr.h;
        paramName: "calibr"
        operation: "Калибровка"
        programIndex: tab.programIndex
    }

    ParamLabel {
        x: iGr.gx * 4 + iGr.w * 3; y: iGr.gyc + iGr.gy * 1 + iGr.h * 0;
        width: iGr.gx * 5 + iGr.w * 4; height: iGr.h;
        text: "Длительность строба (мс)";
        hAlign: Text.AlignLeft
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"analysis")
    }

    ParamInput {
        x: iGr.gx * 8 + iGr.w * 6; y: iGr.gyc + iGr.gy * 1 + iGr.h * 0;
        width: iGr.gx * 1 + iGr.w * 1; height: iGr.h;
        paramName: "analysis"
        programIndex: tab.programIndex
    }

    ParamLabel {
        x: iGr.gx * 8 + iGr.w * 6; y: iGr.gyc + iGr.gy * 1 + iGr.h * 0;
        width: iGr.gx * 3 + iGr.w * 0; height: iGr.h;
        text: "N";
        hAlign: Text.AlignRight
		visible: CodogramsJS.isParamEnabled(tab.programIndex,"nstrobs")
    }

    ParamInput {
        x: iGr.gx * 8 + iGr.w * 7; y: iGr.gyc + iGr.gy * 1 + iGr.h * 0;
        width: iGr.gx * 1 + iGr.w * 1; height: iGr.h;
        paramName: "nstrobs"
        programIndex: tab.programIndex
    }

    Button {
        x: iGr.gx * 4 + iGr.w * 3; y: iGr.gyc + iGr.gy * 2 + iGr.h * 1;
        width: iGr.gx * 2 + iGr.w * 2; height: iGr.h;
		pressbutton: true
        operation: "Сброс"
        onClicked: {
			// benchmarking
			// var date0=new Date();
			// var t0=date0.getTime();
			
			CodogramsJS.resetAllParamsAllSubprograms();
			
			// var date1=new Date();
			// var t1=date1.getTime();
			// cppModel.msg("resetAllParamsAllSubprograms() time="+(t1-t0));
			
			cppModel.refreshControls();
		}
    }

    Button {
        id: applyButton
        x: iGr.gxc + iGr.gx * 2 + iGr.w * 1; y: iGr.gyc + iGr.gy * 2 + iGr.h * 1;
        width: iGr.gx * 2 + iGr.w * 3; height: iGr.h;
		pressbutton: true
        operation: "Применить"
        onClicked: {
            // We need to ensure that all TextInput elements loose activeFocus=>apply current value
            applyButton.forceActiveFocus();
            if (!CodogramsJS.applyAllParamsAllSubprograms()) {
				CodogramsJS.resetAllParamsAllSubprograms();
				cppModel.msg("applyButton.onClicked: applyAllParamsAllSubprograms() failed!");
			}
			// unfortunately, we need to refresh linked text input fields. 
			// So need to send refreshControls() signal
			cppModel.refreshControls();
        }
    }

}

