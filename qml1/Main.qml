//https://doc.qt.io/qt-6/qtquickcontrols-index.html
//https://code.qt.io/cgit/qt/qtdeclarative.git/tree/examples/quickcontrols/gallery?h=6.7
//https://doc.qt.io/qt-5/qtwidgets-desktop-screenshot-example.html
//https://doc.qt.io/qt-6/qtmultimedia-screencapture-example.html#running-the-example
import QtCore 6.0
import QtQuick 6.0
import QtQuick.Shapes 6.0
import QtQuick.Dialogs
import QtQuick.Controls 6.0
import QtMultimedia
import QtWebView


import QtQuick.LocalStorage 2.0


ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    background: Rectangle {
        color:    "Green"
        //radius : 180
    }

    WebView {
             anchors.fill: parent
             url: "https://rastrwin.ru"
    }

    CaptureSession {
        id: captureSession
        camera: Camera {}
        videoOutput: output
        Component.onCompleted: captureSession.camera.start()
    }
    VideoOutput {
        id: output
        x : 1220
        y : 120
        //anchors.fill: parent
        //anchors.fill:
    }



    MediaPlayer {
        id: player
        //source: "https://file-examples-com.github.io/uploads/20"
        autoPlay: true
        source: "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
        audioOutput: AudioOutput {}
        videoOutput: videoOutput
    }
    MediaPlayer {
        id: player2
        //source: "https://file-examples-com.github.io/uploads/20"
        autoPlay: true
        source: "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
        audioOutput: AudioOutput {}
        videoOutput: videoOutput2
    }
    VideoOutput {
        id: videoOutput
        //anchors.fill: parent
        anchors.margins: 20
    }

    VideoOutput {
        id: videoOutput2
        x : 320
        y : 320
        //anchors.fill: parent
        anchors.margins: 20

        NumberAnimation on rotation { from: 0; to: -360 ; duration: 8000; loops: Animation.Infinite; }
    }
    Component.onCompleted: {
        player.play()
        player2.play()
    }

    Image {
        id: img_my
        source :"file:///C:/projects/git_web/web-interface/front/img/cx195_priv.svg"
        //source :"file:///C:/projects/git_web/web-interface/front/img/grf_sz.svg"
        //fillMode: Image.Stretch
        fillMode: Image.Right
        NumberAnimation on rotation { from: 0; to: 360 ; duration: 8000; loops: Animation.Infinite; }
    }

/*
    Drawer {
        id: drawer
        width: 0.66 * window.width
        height: window.height
        visible: true

        Label {
            text: "Content goes here!"
            anchors.centerIn: parent
        }
    }
*/

    Drawer {
        id: drawer
        //width: Math.min(window.width, window.height) / 3 * 2
        width: window.width
        height: window.height

        Image {
            id: img_my2
            //source :"file:///C:/projects/git_web/web-interface/front/img/cx195_priv.svg"
            source :"file:///C:/projects/git_web/web-interface/front/img/grf_sz.svg" // worked from https://en.wikipedia.org/wiki/File_URI_scheme#Windows
            //source :"file://localhost/C:/projects/git_web/web-interface/front/img/grf_sz.svg" // not work!!
            fillMode: Image.Stretch

            //RotationAnimation on rotation {         to: 360;            duration: root.duration;    loops: Animation.Infinite;        }
             NumberAnimation on rotation { from: 0; to: 360 ; duration: 8000; loops: Animation.Infinite; }
        }

        ListView {
            focus: true
            currentIndex: -1
            anchors.fill: parent
            delegate: ItemDelegate {
            width: parent.width
            text: model.text
            highlighted: ListView.isCurrentItem
            onClicked: {
                drawer.close()
                model.triggered()
            }
        }
        model: ListModel {
            ListElement {
                text: qsTr("Open...")
                triggered: function() { fileOpenDialog.open();}
            }
            ListElement {
                text: qsTr("About...")
                triggered: function() { aboutDialog.open(); }
            }
        }
        ScrollIndicator.vertical: ScrollIndicator { }
       }
    }


    header: ToolBar {
        Flow {
            anchors.fill: parent
            ToolButton {
                text: qsTr("Open")
                icon.name: "document-open"
                onClicked: {

                    //fileOpenDialog.open()
                    drawer.open();
                }
            }
            ToolButton {
                text: qsTr("Save")
                icon.name: "document-save"
                onClicked: fileDialog.open()
            }
            ToolButton {
                text: qsTr("SaveAs")
                icon.name: "document-save-as"
                onClicked: fileOpenDialog.open()
            }

            ToolButton {
                text: qsTr("About")
                icon.name: "document-"
                onClicked: aboutDialog.open()
            }

        }
    }


    menuBar: MenuBar { // in import QtQuick.Controls 2 !! ( __2__ )
        Menu {
            title: qsTr("&File")
               Action { text: qsTr("&New...") }
               Action { text: qsTr("&Open...") }
               Action { text: qsTr("&Save") }
               Action { text: qsTr("Save &As...") }
               MenuSeparator { }
               Action { text: qsTr("&Quit") }
           }
        Menu {
            title: qsTr("&About")
            MenuSeparator { }
            Action { text: qsTr("&About"); onTriggered: aboutDialog.open();  }
        }
    }

/*
    menuBar: MenuBar{
        Menu {
                title: qsTr("&File")
                Action {
                    text: qsTr("&Open...")
                    icon.name: "document-open"
                    onTriggered: fileOpenDialog.open()
                }
        }
    }
*/
    FileDialog {
        id: fileOpenDialog
        title: "Select an image file"

        currentFolder:  StandardPaths.writableLocation(StandardPaths.DocumentsLocation )
        nameFilters: [
        "Image files (*.png *.jpeg *.jpg)",]
        onAccepted: {image.source = fileOpenDialog.fileUrl }
    }

    Dialog { // in import QtQuick.Controls 2 !! ( __2__ )
        id: aboutDialog
        title: qsTr("About")
        width: 300
        height: 300
        x: parent.width/2 - width/2
        y: parent.height/2 - height/2
        parent: Overlay.overlay
        //modal: true
        font.bold: true

        Label {
            anchors.fill: parent
            text: qsTr("AAbout \n dfsdfasdfsdfsd \n !!!!!")
            horizontalAlignment: Text.AlignHCenter
        }

        header: ToolBar {
            Flow {
                anchors.fill: parent
                ToolButton {
                    text: qsTr("Open")
                    icon.name: "document-open"
                    onClicked: fileOpenDialog.open()
                }

                ToolButton {
                    text: qsTr("Save")
                    icon.name: "document-open"
                    onClicked: fileOpenDialog.open()
                }

            }
        }

/*
        menuBar: MenuBar { // in import QtQuick.Controls 2 !! ( __2__ )
            Menu {
                title: qsTr("&File")
                   Action { text: qsTr("&New...") }
                   Action { text: qsTr("&Open...") }
                   Action { text: qsTr("&Save") }
                   Action { text: qsTr("Save &As...") }
                   MenuSeparator { }
                   Action { text: qsTr("&Quit") }
               }
            Menu {
                title: qsTr("&About")
                MenuSeparator { }
                Action { text: qsTr("&About"); onTriggered: aboutDialog.open();  }
            }
        }
*/
        standardButtons:  Dialog.Cancel |  Dialog.Ok
        //StandardButton : StandardButton.Ok
    }


    /*FileDialog2 {
        id: fileDialog2
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        onAccepted: image.source = selectedFile
    }*/




    Image {
        id: image

        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        asynchronous: true
    }
}

/*
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")


    Rectangle {
        id: rect1
        x: 12; y: 12
        width: 76; height: 96
        color: "lightsteelblue"
        MouseArea {
            id: area
            width: parent.width
            height: parent.height
            onClicked: rect2.visible = !rect2.visible
        }
    }
    Rectangle {
        id: rect2
        x: 112; y: 12
        width: 76; height: 96
        border.color: "lightsteelblue"
        border.width: 4
        radius: 8
    }


    Text {
        id: label
        x: 120; y: 240


        font.family: "Ubuntu"
        font.pixelSize: 28
        // red sunken text styling
        style: Text.Sunken
        styleColor: '#FF0000'


        property int  ddd: 10000
        NumberAnimation on rotation { from: 0; to: -360; duration: label.ddd ; loops: Animation.Infinite; }
        // custom counter property for space presses
        property int spacePresses: 0
        text: "Space pressed: " + spacePresses + " times"
        // (1) handler for text changes. Need to use function to ca
        onTextChanged: function(text) {
            console.log("text changed to:", text)
        }
        // need focus to receive key events
        focus: true
        // (2) handler with some JS
        Keys.onSpacePressed: {
            increment()
        }
        // clear the text on escape
        Keys.onEscapePressed: {
            label.text = ''
        }
        // (3) a JS function
        function increment() {
            spacePresses = spacePresses + 1
            ddd *= 1000
            label.text = ddd
        }
    }


}
*/
