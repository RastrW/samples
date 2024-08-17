import QtQuick 2.15

Rectangle {

    property string newText: "Hello"
    property QtObject message

    id: root

    width: 300
    height: 300
    color: "green"

    Rectangle {
        property int d: 100
        id: square
        width: d
        height: d
        anchors.centerIn: parent
        color: root.rectColor
        NumberAnimation on rotation { from: 0; to: 360 ; duration: 8000; loops: Animation.Infinite; }
    }


    Text {
        id: text
        anchors.fill: parent
        text: newText
        font.pixelSize: 64
        color: "blue"
        NumberAnimation on rotation { from: 0; to: -360; duration: 10000; loops: Animation.Infinite; }
    }

    Connections {
        target: message

        function onUpdated(msg) {
            newText = msg
            console.log(msg)
        }
    }
}
