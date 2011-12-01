.pragma library


function sendPreedit() {

}

function sendCommit() {

}

function activateActionKey() {

}

function userHide() {
    console.log("Hide VKB")
    return false
}

var actionKeyOverride = {};
actionKeyOverride.icon = "icon-m-input-methods-enter.svg"
actionKeyOverride.label = "" // Or this one, but then disable the icon "Enter"
