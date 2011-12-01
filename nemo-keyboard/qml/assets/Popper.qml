import QtQuick 1.0
import "KeyboardUiConstants.js" as UI

Image {
    id: popper
    visible: false
    source: "popper.png"
    property alias text: popperText.text
    property bool pressed: false
    Text {
        id: popperText
        text: ""
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: "sans"
        font.pixelSize: UI.FONT_SIZE_POPPER
        font.bold: true
        color: UI.TEXT_COLOR_POPPER
    }

    onPressedChanged: {
        if (pressed) {
            popper.visible = true
        } else {
            animatePopperFade.start()
        }
    }
    Timer {
        id: animatePopperFade
        interval: 50
        running: false
        repeat: false
        onTriggered: popper.visible = false
    }

}
