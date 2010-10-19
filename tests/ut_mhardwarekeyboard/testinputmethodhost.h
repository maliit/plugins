#ifndef TESTINPUTMETHODHOST_H
#define TESTINPUTMETHODHOST_H

#include <mabstractinputmethodhost.h>
#include <QString>
#include <QKeyEvent>

/*!
 * \brief Dummy input method host for ut_mhardwarekeyboard
 */
class TestInputMethodHost : public MAbstractInputMethodHost
{
    Q_OBJECT

public:
    TestInputMethodHost()
        : lastKeyEventM(QEvent::KeyRelease, 0, Qt::NoModifier),
          keyEventCounter(0)
    {
    }

    // Methods we don't care about...........................................

    virtual int contentType(bool &/* valid */)
    {
        return 0;
    }

    virtual bool correctionEnabled(bool &/* valid */)
    {
        return false;
    }

    virtual bool predictionEnabled(bool &/* valid */)
    {
        return false;
    }

    virtual bool autoCapitalizationEnabled(bool &/* valid */)
    {
        return false;
    }

    virtual bool surroundingText(QString &/* text */, int &/* cursorPosition */)
    {
        return false;
    }

    virtual bool hasSelection(bool &/* valid */)
    {
        return false;
    }

    virtual int inputMethodMode(bool &/* valid */)
    {
        return 0;
    }

    virtual QRect preeditRectangle(bool &/* valid */)
    {
        return QRect();
    }

    virtual void notifyImInitiatedHiding()
    {
    }

    virtual void copy()
    {
    }

    virtual void paste()
    {
    }

    virtual void setRedirectKeys(bool /* enabled */)
    {
    }

    virtual void setDetectableAutoRepeat(bool /* enabled */)
    {
    }

    virtual void setGlobalCorrectionEnabled(bool)
    {
    }

    // Methods we care about.................................................

    virtual void sendPreeditString(const QString &string,
                                   MInputMethod::PreeditFace /* preeditFace */)
    {
        lastPreeditStringM = string;
    }

    virtual void sendCommitString(const QString &string)
    {
        lastCommitStringM = string;
    }

    virtual void sendKeyEvent(const QKeyEvent &keyEvent,
                              MInputMethod::EventRequestType requestType)
    {
        Q_UNUSED(requestType);
        ++keyEventCounter;
        lastKeyEventM = keyEvent;
    }

    // Special methods for ut_mhardwarekeyboard............................

    QKeyEvent lastKeyEvent() const
    {
        return lastKeyEventM;
    }

    QString lastPreeditString() const
    {
        return lastPreeditStringM;
    }

    QString lastCommitString() const
    {
        return lastCommitStringM;
    }

    unsigned int keyEventsSent() const
    {
        return keyEventCounter;
    }

    virtual void setInputModeIndicator(MInputMethod::InputModeIndicator /*indicator*/)
    {
    }

    virtual void switchPlugin(MInputMethod::SwitchDirection /*direction*/)
    {
    }

    virtual void switchPlugin(const QString & /*pluginName*/)
    {
    }

    virtual void showSettings()
    {
    }

private:
    Q_DISABLE_COPY(TestInputMethodHost)

    QString lastPreeditStringM;
    QString lastCommitStringM;
    QKeyEvent lastKeyEventM;
    unsigned int keyEventCounter;
};

#endif