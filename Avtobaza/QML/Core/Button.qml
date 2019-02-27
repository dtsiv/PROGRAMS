/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 1.0
import "codograms.js" as CodogramsJS

BorderImage {
    id: button

    property alias operation: buttonText.text
    property string color: ""
	property bool pressbutton: false
    signal clicked
    signal rightClicked(string operation)

    source: "images/button-" + color + ".png"; clip: true
    border { left: 10; top: 10; right: 10; bottom: 10 }

    Rectangle {
        id: shade
        anchors.fill: button; radius: 10; color: "black"; opacity: 0
    }

    Text {
        id: buttonText
        anchors.centerIn: parent; anchors.verticalCenterOffset: -1
        font.pixelSize: parent.width > parent.height ?  
                        Math.min(parent.height * .4, 20) :  
                        Math.min(parent.width * .4, 20)
        style: Text.Sunken; color: "white"; styleColor: "black"; smooth: true
		elide: Text.ElideRight;
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
		acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            //doOp(operation)
            button.forceActiveFocus();
            button.clicked();
        }
	onPressed: {
	    if (mouse.buttons==Qt.RightButton) {
		    // suppress mouse event
		    mouse.accepted=false;
		    // signal about right mouse click on the button
		    button.rightClicked(operation);
            }
	}
    }

    states: [
        State {
            name: "pressed"; when: button.pressbutton && mouseArea.pressed == true && mouseArea.pressedButtons == Qt.LeftButton
            PropertyChanges { target: shade; opacity: .4 }
        },
        State {
            name: "checked"
            PropertyChanges { target: shade; opacity: .6 }
        }
    ]
}


