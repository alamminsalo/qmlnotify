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
 setup* Feel free to modify or completely reimplement your own
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


        else 
			root.width = 240 //Shorten the notification a bit

        //Setup timer
        timer.interval = properties.timeout !== -1 ? properties.timeout : 5000

		setup();
    }

    function setup() {
		// Setup initial position
		setPosition()

		function startAll() {
				root.visible = true
				show();
				anim.start();
				timer.start()
		}

		//Show after image has loaded (or failed to load)
		if (img.status === Image.Loading) {
			img.statusChanged.connect(function (){
				startAll();
			});
		}

		//If image is not loading, show right away
		else {
			startAll();
		}
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
    property string location: 'topright'
    function setPosition(){
        if (location === 'topleft') {
            x =  50;
            y = -height;
            destY = 50;
		}

		else if (location === 'topright') {
            x = Screen.width - width  - 50;
            y = -height;
            destY = 50;
		}

		else if (location === 'bottomleft') {
            x = 50;
            y = Screen.height;
            destY = Screen.height - height  - 50;
		}

		else if (location === 'bottomright') {
            x = Screen.width - width  - 50;
            y = Screen.height;
            destY = Screen.height - height - 50;
		}

		else if (location === 'topcenter') {
			x = Screen.width / 2 - width / 2;
            y = -height
            destY = 50;
		}
    }

	// Slide-in animation
    NumberAnimation {
        id: anim
        target: root
        properties: "y"
        from: y
        to: destY
        duration: 300
        easing.type: Easing.OutBack
    }

	// Actual notification part
    Rectangle {
        id: baserect
        anchors.fill: parent

		color: "#222"
        clip: true

        Item {
            anchors.fill: parent
            anchors.margins: parent.border.width

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
					width: parent.width * 0.8
					height: width
					fillMode: Image.PreserveAspectCrop

					Behavior on width {
						NumberAnimation {
							duration: 350
							easing.type: Easing.OutCubic
						}
					}
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

            Rectangle {
                id: progress
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                }
                height: 4
                width: root.visible ? baserect.width : 0
                color: "white"

                Behavior on width {
                    NumberAnimation {
                        duration: timer.interval
                    }
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
