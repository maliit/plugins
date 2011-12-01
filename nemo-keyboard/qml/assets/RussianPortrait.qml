import QtQuick 1.0
import "KeyboardUiConstants.js" as UI



//й ц у к е н г ш щ з х
//ф ы в а п р о л д ж э
//я ч с м и т ь б ю
//ъ

Column {
    anchors.fill: parent
    anchors.topMargin: 8
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: 16
    property variant row1: ["й1€", "ц2£", "у3$", "к4¥", "е5₹", "н6%", "г7<", "ш8>", "щ9[", "з0]", "х0]"]
    property variant row2: ["ф*`", "ы#^", "в+|", "а-_", "п=§", "р{}", "о?¿", "л!¡", "д!¡", "ж!¡", "э!¡"]
    property variant row3: ["я@«", "ч~»", "с/\"", "м\\“", "и'”", "т;„", "ь:&", "б;„", "ю:&"]
    property variant accents_row1: ["", "", "eèéêë", "", "tþ", "yý", "uûùúü", "iîïìí", "oöôòó", ""]
    property variant accents_row2: ["aäàâáãå", "", "dð", "", "", "", "", "", ""]
    property variant accents_row3: ["", "", "cç", "", "", "nñ", ""]

    property int columns: Math.max(row1.length, row2.length, row3.length)
    property int keyWidth: (columns == 11) ? UI.portraitWidthNarrow : UI.portraitWidth
    property int keyHeight: UI.portraitHeight
    property int keyMargin: (columns == 11) ? UI.portraitMarginNarrow : UI.portraitMargin
    property bool isShifted: false
    property bool isShiftLocked: false
    property bool inSymView: false
    property bool inSymView2: false

    property string layoutName: "Русский"

    Row { //Row 1
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: keyMargin
        Repeater {
            model: row1
            CharacterKey {
                width: keyWidth; height: keyHeight
                sizeType: "keyboard-key-38x60.png"
                caption: row1[index][0]
                captionShifted: row1[index][0].toUpperCase()
                symView: row1[index][1]
                symView2: row1[index][2]
            }
        }
    }

    Row { //Row 2
        anchors.horizontalCenter: parent.horizontalCenter

        spacing: keyMargin
        Repeater {
            model: row2
            CharacterKey {
                width: keyWidth; height: keyHeight
                sizeType: "keyboard-key-38x60.png"
                caption: row2[index][0]
                captionShifted: row2[index][0].toUpperCase()
                symView: row2[index][1]
                symView2: row2[index][2]
            }
        }
    }

    Row { //Row 3
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: (columns == 11) ? 5 : 16
        FunctionKey {
            width: UI.PORTRAIT_NARRROW_SHIFT_WIDTH; height: keyHeight
            sourceWidth: 24
            sourceHeight: 24
            fontSize: UI.FONT_SIZE_RUSSIAN
            icon: inSymView ? ""
                            : (isShiftLocked) ? "icon-m-input-methods-capslock.svg"
                                              : (isShifted) ? "icon-m-input-methods-shift-uppercase.svg"
                                                            : "icon-m-input-methods-shift-lowercase.svg"

            caption: inSymView ? (inSymView2 ? "2/2" : "1/2") : ""

            onClickedPass: {
                if (inSymView) {
                    inSymView2 = !inSymView2
                } else {
                    isShifted = (!isShifted)
                    isShiftLocked = false
                }
            }
            onPressedAndHoldPass: {
                if (!inSymView) {
                    isShifted = true
                    isShiftLocked = true
                }
            }
        }
        Row {
            spacing: keyMargin
            Repeater {
                model: row3
                CharacterKey {
                    width: keyWidth; height: keyHeight
                    sizeType: "keyboard-key-38x60.png"
                    caption: row3[index][0]
                    captionShifted: row3[index][0].toUpperCase()
                    symView: row3[index][1]
                    symView2: row3[index][2]
                }
            }
        }
        FunctionKey {
            width: UI.PORTRAIT_NARRROW_SHIFT_WIDTH; height: keyHeight
            icon: "icon-m-input-methods-backspace.svg"
            sourceWidth: 24
            sourceHeight: 24
            onClickedPass: { MInputMethodQuick.sendCommit("\b"); }
        }
    }

    Row { //Row 4
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: (columns == 11) ? 5 : 16
        FunctionKey {
            width: UI.PORTRAIT_NARROW_OTT_WIDTH; height: keyHeight
            caption: "?123"
            fontSize: UI.FONT_SIZE_RUSSIAN
            onClickedPass: { inSymView = (!inSymView) }
        }
        Row {
            spacing: 5
            CharacterKey { caption: "-"; captionShifted: "-"; width: 38; height: keyHeight; sizeType: "keyboard-key-38x60.png" }
            CharacterKey { caption: ","; captionShifted: ","; width: 38; height: keyHeight; sizeType: "keyboard-key-38x60.png" }
            CharacterKey { caption: " "; width:104 ; height: keyHeight; sizeType: "keyboard-key-104x60.png" }
            CharacterKey { caption: "."; captionShifted: "."; width: 38; height: keyHeight; sizeType: "keyboard-key-38x60.png" }
            CharacterKey { caption: "ъ"; captionShifted: "Ъ"; width: 38; height: keyHeight; sizeType: "keyboard-key-38x60.png" }
        }
        FunctionKey {
            width: 126; height: keyHeight
            icon: MInputMethodQuick.actionKeyOverride.icon
            caption: MInputMethodQuick.actionKeyOverride.label
            onReleased: {
                MInputMethodQuick.activateActionKey()
            }
        }
    }
}

