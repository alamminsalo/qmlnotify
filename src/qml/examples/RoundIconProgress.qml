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
import QtGraphicalEffects 1.0
import "."

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
    width: 160
    height: width
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

        //Setup timer
        timer.interval = properties.timeout !== -1 ? properties.timeout : 5000

		progress.animationDuration = timer.interval

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
        onTriggered: exitAnim.start()
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

    function setup() {
		// Setup initial position
		setPosition('topright')

		function run() {
			root.visible = true;
			show();
			timer.start()
			progress.arcBegin = 360
		}

		//Show after image has loaded (or failed to load)
		if (img.status === Image.Loading) {
			img.statusChanged.connect(function (){
				run();
			});
		}

		//If image is not loading, show right away
		else {
			run();
		}
    }

    // Slide-out animation
    NumberAnimation {
        id: exitAnim
        target: baserect
        properties: "width"
        to: 0
        duration: 270
        easing.type: Easing.OutCubic

		onStarted: progress.destroy()
        onStopped: timeout()
    }

    // Actual notification part
    Rectangle {
        id: baserect
        anchors.centerIn: parent

        width: parent.visible ? parent.width * 0.77 : 0
        height: width

        color: "#222"
        radius: height / 2

        visible: parent.visible
        //clip: true

        Behavior on width {
            NumberAnimation {
                duration: 400
                easing.type: Easing.OutBack
            }
        }

        //border {
        //    width: 5
        //    color: "white"
        //}

        Behavior on width {
            NumberAnimation {
                duration: 350
                easing.type: Easing.OutCubic
            }
        }

        AnimatedImage {
            id: img
            anchors.centerIn: parent
            width: parent.width
            height: width
            fillMode: Image.PreserveAspectCrop
            smooth: true
            visible: false

			//Rectangle {
			//	anchors.fill: parent
			//	color: "#000"
			//	opacity: 0.333
			//}

        }

        OpacityMask {
            anchors {
                fill: parent
                //margins: parent.border.width
            }
            source: img
            maskSource: parent

            Item {

                anchors {
                    fill: parent
                    topMargin: 14
                    bottomMargin: 14
                    leftMargin: 10
                    rightMargin: 10
                }

                Text {
                    id: titleText
                    anchors {
                        left: parent.left
                        top: parent.top
                        right: parent.right
                        bottom: parent.verticalCenter
						bottomMargin: 5
                    }
                    fontSizeMode: Text.Fit
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: "white"
                    wrapMode: Text.Wrap
                    font.bold: true
                    font.pixelSize: 14
                    font.family: fontloader.name
                    style: Text.Raised; styleColor: "black"
					elide: Text.ElideRight
					renderType: Text.NativeRendering
					font.hintingPreference: Font.PreferFullHinting
                }

                Text {
                    id: bodyText
                    anchors {
                        top: titleText.bottom
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    fontSizeMode: Text.Fit
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: "white"
                    wrapMode: Text.Wrap
                    font.pixelSize: 14
                    style: Text.Raised; styleColor: "black"
					elide: Text.ElideRight
					renderType: Text.NativeRendering
					font.hintingPreference: Font.PreferFullHinting
                }
            }

			ProgressCircle {
				id: progress
				anchors.fill: parent
				anchors.margins: -1
				colorCircle: "#eee"
				colorBackground: "#E6E6E6"
				arcBegin: 0
				arcEnd: 360
				lineWidth: 3
			}
        }

        MouseArea {
            id: mArea
            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                exitAnim.start()
            }
        }
    }
}
