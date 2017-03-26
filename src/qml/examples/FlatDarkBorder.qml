/*
 * Copyright © 2017 Antti Lamminsalo
 *
 * This file is part of qmlnotify.
 *
 * qmlnotify is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with qmlnotify.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.5
import QtQuick.Window 2.0

/*
 * Reference implementation for qml notification.
 * Feel free to modify or completely reimplement your own
 */
Window {
	//  notification data entries:
	//
	//  app_name -> application name
	//  app_id -> numeral id
	//  summary -> title text
	//  body -> message body text
	//  icon -> message icon/img path
	//  timeout -> message timout in ms
	//  actions -> see org.freedesktop.Notifications
	//  hints -> see org.freedesktop.Notifications
	//
	//  Non-standard variables:
	//  image_data -> base64 encoded png data passed via hints.image_data
	//  app_icon -> base64 encoded png data from icon theme
	//
	//  Usage: var appname = properties.app_name
	property var properties

	// Timeout signal, needs to be implemented!
	signal timeout()

	//Usually needed
	title: "Notification"
	flags: Qt.SplashScreen | Qt.NoFocus | Qt.X11BypassWindowManagerHint | Qt.BypassWindowManagerHint | Qt.WindowStaysOnTopHint | Qt.Popup



	//======== Rest is reference //==============================================
	id: root
	width: 300
	height: 100
	color: "transparent"

	property int destY: 0

	onPropertiesChanged: {
		//Setup data
		titleText.text = properties.summary
		bodyText.text = properties.body

		//Setup icon: icon > image_data > app_icon
		if (properties.app_icon)
		img.source = properties.app_icon

		else if (properties.icon)
		img.source = properties.icon

		else if (properties.image_data)
		img.source = properties.image_data


		else root.width = 240 //Shorten the notification a bit

		//Setup timer
		timer.interval = properties.timeout !== -1 ? properties.timeout : 5000

		setup();
	}

	// Use Noto Sans system font
	FontLoader {
		id: fontloader
		name: "Noto Sans"
	}

	// Triggers timeout 
	Timer {
		id: timer
		onTriggered: timeout()
	}

	// Location setup
	function setPosition(location){
		if (location === 'topleft') {
			x =  50;
			y = 50;
		}

		else if (location === 'topright') {
			x = Screen.width - width  - 50;
			y = 50;
		}

		else if (location === 'bottomleft') {
			x = 50;
			y = Screen.height - height  - 50;
		}

		else if (location === 'bottomright') {
			x = Screen.width - width  - 50;
			y = Screen.height - height - 50;
		}

		else if (location === 'topcenter') {
			x = Screen.width / 2 - width / 2;
			y = 50;
		}
	}

	//Triggers when visible is set to true
	function setup() {
		// Setup initial position
		setPosition('topright')

		function doShow() {
			show();
			baserect.grow()
			timer.start()
		}

		//Show after image has loaded (or failed to load)
		if (img.status === Image.Loading) {
			img.statusChanged.connect(function (){
				doShow();
			});
		}

		//If image is not loading, show right away
		else {
			doShow();
		}
	}

	// Actual notification part
	Rectangle {
		id: baserect
		anchors.centerIn: parent

		color: "#222"
		clip: true

		border {
			width: 4
			color: "#E5E9F0"
		}

		// Initial height, width
		width: 0
		height: 0

		function grow() {
			width = root.width
			height = root.height
		}

		Behavior on width {
			NumberAnimation {
				duration: 200
				easing.type: Easing.OutCubic
			}
		}
		Behavior on height {
			NumberAnimation {
				duration: 400
				easing.type: Easing.InCirc
			}
		}

		Item {
			anchors {
				fill: parent
				topMargin: 4
				bottomMargin: 4
				leftMargin: 8
				rightMargin: 4
			}

			Item {
				//Image wrapper
				id: imgwrap

				anchors {
					top: parent.top
					bottom: parent.bottom
					left: parent.left
				}
				width: img.status == Image.Ready ? parent.height * 0.9 : 0

				AnimatedImage {
					id: img
					anchors.centerIn: parent
					width: parent.width * 0.95
					height: width
					fillMode: Image.PreserveAspectCrop
				}
			}

			Item {

				anchors {
					left: imgwrap.right
					right: parent.right
					top: parent.top
					bottom: parent.bottom
					margins: 10
				}

				Text {
					id: titleText
					anchors {
						left: parent.left
						top: parent.top
						right: parent.right
					}
					verticalAlignment: Text.AlignVCenter
					color: "white"
					wrapMode: Text.WordWrap
					font.bold: true
					font.pixelSize: 14
					font.family: fontloader.name
				}

				Text {
					id: bodyText
					anchors {
						top: titleText.bottom
						topMargin: 4
						left: parent.left
						right: parent.right
						bottom: parent.bottom
					}
					verticalAlignment: Text.AlignVCenter
					color: "white"
					wrapMode: Text.WordWrap
					font.pixelSize: 13
					font.family: fontloader.name
					elide: Text.ElideRight
				}
			}

			MouseArea {
				id: mArea
				anchors.fill: parent
				hoverEnabled: true

				onClicked: {
					timeout()
				}
			}
		}
	}
}
