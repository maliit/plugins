// License would be CC SA 2.5
// Taken from http://qt-project.org/wiki/Spell_Checking_with_Hunspell#b8c5300cf30231da0a9520d8bde7120d

#ifndef MALIIT_KEYBOARD_SPELLCHECKER_H
#define MALIIT_KEYBOARD_SPELLCHECKER_H

#include <QtCore>

class Hunspell;

namespace MaliitKeyboard {
namespace Logic {

class SpellChecker
{
public:
    // FIXME: Find better way to discover default dictionaries.
    // FIXME: Allow changing languages in between.
    explicit SpellChecker(const QString &dictionaryPath = QString("/usr/share/hunspell/en_GB"),
                          const QString &userDictionary = QString("%1/.config/hunspell/en_GB.dic").arg(QDir::homePath()));

    ~SpellChecker();

    bool spell(const QString &word);
    QStringList suggest(const QString &word,
                        int limit = -1);
    void ignoreWord(const QString &word);
    void addToUserWordlist(const QString &word);

private:
    void putWord(const QString &word);
    QScopedPointer<Hunspell> m_hunspell;
    QString m_user_dictionary_path;
    QString m_encoding;
    QTextCodec *m_codec;
};

}} // namespace Logic, MaliitKeyboard

#endif // MALIIT_KEYBOARD_SPELLCHECKER_H
