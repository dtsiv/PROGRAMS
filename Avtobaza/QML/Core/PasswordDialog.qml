import QtQuick 1.0

Rectangle {
	visible: true
	
	anchors { left: buttonsPanel.right; top: parent.top; bottom: parent.bottom; right: parent.right;}
	
	Text {
		font.pixelSize:	20
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
		width: passwordInput.width; height: passwordInput.height
		x: parent.width/10; y: passwordInput.y;
		text: "Введите пароль!"
	}
	
	BorderImage {
		source: "images/lineedit-bg.png"
		width: passwordInput.width+20; height: passwordInput.height+20
		border { left: 4; top: 4; right: 4; bottom: 4 }
		x: parent.width/10 + passwordInput.width-10; y: passwordInput.y-10;
	}
	
	TextInput {
		id: passwordInput
                echoMode: TextInput.Password
		font.pixelSize:	30
		width: 4*parent.width/10;
		x: parent.width/10 + passwordInput.width; y: parent.height/2-30;
	}
	
	Button {
		x: parent.width/10; 
		y: passwordInput.y + passwordInput.height + 40;
		width: 2*passwordInput.width+10;
		operation: "Продолжить"
		onClicked: {
			var checkSum=0;
			var passwd=passwordInput.text;
			for (var i=0; i<passwd.length; i++) {
				checkSum+=passwd.charCodeAt(i);
			}
			if (checkSum==6471) {
				passwdDialog.visible=false;
				tabWidget.visible=true;
				buttonsPanel.visible=true;
			}
		}
	}
}
