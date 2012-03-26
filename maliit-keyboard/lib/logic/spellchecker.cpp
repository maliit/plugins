// License would be CC SA 2.5
// Taken from http://qt-project.org/wiki/Spell_Checking_with_Hunspell#b8c5300cf30231da0a9520d8bde7120d

#include "spellchecker.h"

#ifdef HAVE_HUNSPELL
#include "hunspell/hunspell.hxx"
#else
class Hunspell
{};
#endif

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QStringList>
#include <QDebug>

namespace MaliitKeyboard {
namespace Logic {

SpellChecker::SpellChecker(const QString &dictionaryPath,
                           const QString &userDictionary)
{
#ifndef HAVE_HUNSPELL
    Q_UNUSED(dictionaryPath)
    Q_UNUSED(userDictionary)
#else
    m_user_dictionary_path = userDictionary;

    QString dictFile = dictionaryPath + ".dic";
    QString affixFile = dictionaryPath + ".aff";
    QByteArray dictFilePathBA = dictFile.toLocal8Bit();
    QByteArray affixFilePathBA = affixFile.toLocal8Bit();
    m_hunspell.reset(new Hunspell(affixFilePathBA.constData(), dictFilePathBA.constData()));

    // detect encoding analyzing the SET option in the affix file
    m_encoding = "ISO8859-1";
    QFile _affixFile(affixFile);
    if (_affixFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&_affixFile);
        QRegExp enc_detector("^\\s*SET\\s+([A-Z0-9\\-]+)\\s*", Qt::CaseInsensitive);
        for(QString line = stream.readLine(); !line.isEmpty(); line = stream.readLine()) {
            if (enc_detector.indexIn(line) > -1) {
                m_encoding = enc_detector.cap(1);
                qDebug() << QString("Encoding set to ") + m_encoding;
                break;
            }
        }
        _affixFile.close();
    }
    m_codec = QTextCodec::codecForName(this->m_encoding.toLatin1().constData());

    if(!m_user_dictionary_path.isEmpty()) {
        QFile userDictonaryFile(m_user_dictionary_path);
        if(userDictonaryFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&userDictonaryFile);
            for(QString word = stream.readLine(); !word.isEmpty(); word = stream.readLine())
                putWord(word);
            userDictonaryFile.close();
        } else {
            qWarning() << "User dictionary in " << m_user_dictionary_path << "could not be opened";
        }
    } else {
        qDebug() << "User dictionary not set.";
    }
#endif
}

bool SpellChecker::spell(const QString &word)
{
#ifndef HAVE_HUNSPELL
    Q_UNUSED(word)
    return true;
#else
    // Encode from Unicode to the encoding used by current dictionary
    return m_hunspell->spell(m_codec->fromUnicode(word).constData()) != 0;
#endif
}

QStringList SpellChecker::suggest(const QString &word,
                                  int limit)
{
#ifndef HAVE_HUNSPELL
    Q_UNUSED(word)
    Q_UNUSED(limit)
    return QStringList();
#else
    if (spell(word)) {
        return QStringList();
    }

    char **suggestions_buf;
    QStringList suggestions;

    // Encode from Unicode to the encoding used by current dictionary
    const int num_suggestions = m_hunspell->suggest(&suggestions_buf,
                                                    m_codec->fromUnicode(word).constData());
    const int count(limit == -1 ? num_suggestions
                                : qMin<int>(limit, num_suggestions));

    for (int index = 0; index  < count; ++index) {
        suggestions.append(m_codec->toUnicode(suggestions_buf[index]));
        free(suggestions_buf[index]);
    }

    return suggestions;
#endif
}

SpellChecker::~SpellChecker()
{}

void SpellChecker::ignoreWord(const QString &word)
{
    putWord(word);
}

void SpellChecker::putWord(const QString &word)
{
#ifndef HAVE_HUNSPELL
    Q_UNUSED(word)
#else
    m_hunspell->add(m_codec->fromUnicode(word).constData());
#endif
}

void SpellChecker::addToUserWordlist(const QString &word)
{
#ifndef HAVE_HUNSPELL
    Q_UNUSED(word)
#else
    putWord(word);
    if(!m_user_dictionary_path.isEmpty()) {
        QFile userDictonaryFile(m_user_dictionary_path);
        if(userDictonaryFile.open(QIODevice::Append)) {
            QTextStream stream(&userDictonaryFile);
            stream << word << "\n";
            userDictonaryFile.close();
        } else {
            qWarning() << "User dictionary in " << m_user_dictionary_path << "could not be opened for appending a new word";
        }
    } else {
        qDebug() << "User dictionary not set.";
    }
#endif
}

}} // namespace Logic, MaliitKeyboard
