import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root

    property string newText: "Hello"
    property QtObject message
    property  string  rectColor: "green"
    property  int duration: 10000
    property int running : 10

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
    //
    Button { // our Button component
        id: button
        property int counter : 0
        x: 12; y: 12
        text: "Start"
        onClicked: {
            text.text = "Button clicked!" + counter + " times"
            text.scale = counter/2
            counter += 1
        }
        //NumberAnimation on rotation { from: 0; to: 360 ; duration: 8000; loops: Animation.Infinite; }
        RotationAnimation on rotation {
            to: 360
            duration: root.duration
        //    running: root.running
            loops: Animation.Infinite;
        }
    }
    //TextInput {
    TextEdit{
        id: input1
        x: 38; y: 128
        width: 96; height: 20
        color:"blue"
        focus: true
        text: "Text Input 1"

        RotationAnimation on rotation {
            to: 360
            duration: 10*root.duration
            running: root.running
            loops: Animation.Infinite;
        }
    }
    Image {
        x: 12+64+12; y: 12
        // width: 72
        height: 72/2
        // NOT WORK!!
        //source: "cx195_priv_ink.svg"
        //source: QUrl::fromLocalFile("C:\projects\git_web\web-interface\front\img\cx195_priv_ink.svg")
        //source: QUrl::fromLocalFile("C:/projects/git_web/web-interface/front/img/cx195_priv_ink.svg")
        //source: encodeURIComponent("C:/projects/git_web/web-interface/front/img/cx195_priv_ink.svg")
        fillMode: Image.PreserveAspectCrop
        clip: true
    }
    Text {
        id: text
        anchors.fill: parent
        text: newText
        font.pixelSize: 32
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
