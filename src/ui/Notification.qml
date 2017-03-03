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
import QtQuick.Layouts 1.1

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
    //  icon -> message icon/img
    //  timeout -> message timout in ms
    //  actions -> see org.freedesktop.Notifications
    //  hints -> see org.freedesktop.Notifications
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
    width: 400
    height: 120
    color: "transparent"

    //Locations: 0 - topleft, 1 - topright, 2 - bottomleft, 3 - bottomright
    property int location: 1
    property int destY: 0

    onPropertiesChanged: {
        //Setup data
        titleText.text = properties.summary
        bodyText.text = properties.body
        img.source = properties.icon

        timer.interval = properties.timeout !== -1 ? properties.timeout : 5000
        timer.start()

        titleText.font.pixelSize = 16
        bodyText.font.pixelSize = 13
    }

    Timer {
        id: timer
        onTriggered: timeout()
    }

    function setPosition(){
        switch (location){
        case 0:
            x =  50
            y = -height
            destY = 50
            break

        case 1:
            x = Screen.width - width  - 50
            y = -height
            destY = 50
            break

        case 2:
            x = 50
            y = Screen.height
            destY = Screen.height - height  - 50
            break

        case 3:
            x = Screen.width - width  - 50
            y = Screen.height
            destY = Screen.height - height - 50
            break
        }
    }

    onVisibleChanged: {
        if (visible){
            setPosition()
            show()
            //raise()
            anim.start()
        }
    }

    NumberAnimation {
        id: anim
        target: root
        properties: "y"
        from: y
        to: destY
        duration: 300
        easing.type: Easing.OutCubic
    }

    Rectangle {
        id: baserect
        anchors.fill: parent

        color: "#222"
        clip: true
        //radius: 10

        border {
            width: 4
            color: "white"
        }

        Image {
            id: img
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
                right: parent.horizontalCenter
                rightMargin: 20
                margins: 4
            }

            fillMode: Image.Stretch
        }

        ColumnLayout {

            anchors {
                left: parent.horizontalCenter
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }

            anchors {
                top: parent.top
                left: img.right
                bottom: parent.bottom
                right: parent.right
            }

            Text {
                id: titleText
                color: "white"
                wrapMode: Text.WordWrap
                font.bold: true
                font.pixelSize: 16
            }

            Text {
                id: bodyText
                color: "white"
                wrapMode: Text.WordWrap
                font.pixelSize: 14
            }

        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                timeout()
            }
        }
    }
}
