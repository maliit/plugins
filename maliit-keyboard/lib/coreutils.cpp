
#include "coreutils.h"

namespace MaliitKeyboard {
namespace CoreUtils {

const QString &pluginDataDirectory() {
    static QString pluginDataDirectory;

    if (pluginDataDirectory.isNull()) {
        QByteArray envVar = qgetenv("MALIIT_PLUGINS_DATADIR");
        if (envVar.isEmpty()) {
            pluginDataDirectory = QString::fromUtf8(MALIIT_PLUGINS_DATA_DIR);
        } else {
            pluginDataDirectory = QString::fromUtf8(envVar);
        }
    }

    return pluginDataDirectory;
}

const QString &maliitKeyboardDataDirectory() {
    static QString maliitKeyboardDataDirectory;

    if (maliitKeyboardDataDirectory.isNull()) {
        QByteArray envVar = qgetenv("MALIIT_KEYBOARD_DATADIR");
        if (envVar.isEmpty()) {
            maliitKeyboardDataDirectory = QString::fromUtf8(MALIIT_KEYBOARD_DATA_DIR);
        } else {
            maliitKeyboardDataDirectory = QString::fromUtf8(envVar);
        }
    }

    return maliitKeyboardDataDirectory;
}

}
}
