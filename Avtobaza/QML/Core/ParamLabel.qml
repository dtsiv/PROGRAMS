import QtQuick 1.0
import "codograms.js" as CodogramsJS

Rectangle {
    property alias text: label.text;
    property alias hAlign: label.horizontalAlignment;


    Text {
        id: label
		//anchors.left: parent.left
		//anchors.verticalCenter: parent.verticalCenter
		anchors.fill: parent
		
		font.pixelSize: parent.width > parent.height ?  
				Math.min(parent.height * .5, 20) :  
				Math.min(parent.width * .5, 20)
		elide: Text.ElideRight;
		verticalAlignment: Text.AlignVCenter;
		horizontalAlignment: Text.AlignHCenter;
	}
}

