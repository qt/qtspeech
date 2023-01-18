// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only




#ifndef QTEXTTOSPEECH_H
#define QTEXTTOSPEECH_H

#include <QtTextToSpeech/qtexttospeech_global.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>
#include <QtCore/QSharedDataPointer>
#include <QtCore/qlocale.h>
#include <QtTextToSpeech/qvoice.h>

QT_BEGIN_NAMESPACE

class QAudioFormat;

class QTextToSpeechPrivate;
class Q_TEXTTOSPEECH_EXPORT QTextToSpeech : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString engine READ engine WRITE setEngine NOTIFY engineChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(double rate READ rate WRITE setRate NOTIFY rateChanged)
    Q_PROPERTY(double pitch READ pitch WRITE setPitch NOTIFY pitchChanged)
    Q_PROPERTY(QLocale locale READ locale WRITE setLocale NOTIFY localeChanged)
    Q_PROPERTY(QVoice voice READ voice WRITE setVoice NOTIFY voiceChanged)
    Q_PROPERTY(Capabilities engineCapabilities READ engineCapabilities NOTIFY engineChanged)
    Q_DECLARE_PRIVATE(QTextToSpeech)

public:
    enum State {
        Ready,
        Speaking,
        Paused,
        Error,
        Synthesizing
    };
    Q_ENUM(State)

    enum class ErrorReason {
        NoError,
        Initialization,
        Configuration,
        Input,
        Playback
    };
    Q_ENUM(ErrorReason)

    enum class BoundaryHint {
        Default,
        Immediate,
        Word,
        Sentence
    };
    Q_ENUM(BoundaryHint)

    enum class Capability {
        None                = 0,
        Speak               = 1 << 0,
        WordByWordProgress  = 1 << 1,
        Synthesize          = 1 << 2,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities)

    explicit QTextToSpeech(QObject *parent = nullptr);
    explicit QTextToSpeech(const QString &engine, QObject *parent = nullptr);
    explicit QTextToSpeech(const QString &engine, const QVariantMap &params,
                           QObject *parent = nullptr);
    ~QTextToSpeech() override;

    Q_INVOKABLE bool setEngine(const QString &engine, const QVariantMap &params = QVariantMap());
    QString engine() const;
    QTextToSpeech::Capabilities engineCapabilities() const;

    QTextToSpeech::State state() const;
    Q_INVOKABLE QTextToSpeech::ErrorReason errorReason() const;
    Q_INVOKABLE QString errorString() const;

    Q_INVOKABLE QList<QLocale> availableLocales() const;
    QLocale locale() const;

    QVoice voice() const;
    Q_INVOKABLE QList<QVoice> availableVoices() const;

    double rate() const;
    double pitch() const;
    double volume() const;

    Q_INVOKABLE static QStringList availableEngines();

# ifdef Q_QDOC
    template <typename Functor>
    void synthesize(const QString &text, Functor functor);
    template <typename Functor>
    void synthesize(const QString &text, const QObject *context, Functor functor);
# else
    template <typename Slot> // synthesize to a QObject member function
    void synthesize(const QString &text,
        const typename QtPrivate::FunctionPointer<Slot>::Object *receiver, Slot slot)
    {
        using CallbackSignature = QtPrivate::FunctionPointer<void (*)(QAudioFormat, QByteArray)>;
        using SlotSignature = QtPrivate::FunctionPointer<Slot>;

        static_assert(int(SlotSignature::ArgumentCount) <= int(CallbackSignature::ArgumentCount),
            "Slot requires more arguments than what can be provided.");
        static_assert((QtPrivate::CheckCompatibleArguments<typename CallbackSignature::Arguments,
                      typename SlotSignature::Arguments>::value),
            "Slot arguments are not compatible (must be QAudioFormat, QByteArray)");

        auto slotObj = new QtPrivate::QSlotObject<Slot, typename SlotSignature::Arguments, void>(slot);
        synthesizeImpl(text, slotObj, receiver);
    }

    // synthesize to a functor or function pointer (with context)
    template <typename Func, std::enable_if_t<
        !QtPrivate::FunctionPointer<Func>::IsPointerToMemberFunction
        && !std::is_same<const char *, Func>::value, bool> = true>
    void synthesize(const QString &text, const QObject *context, Func func)
    {
        using CallbackSignature = QtPrivate::FunctionPointer<void (*)(QAudioFormat, QByteArray)>;
        constexpr int MatchingArgumentCount = QtPrivate::ComputeFunctorArgumentCount<
            Func, CallbackSignature::Arguments>::Value;

        static_assert(MatchingArgumentCount == 0
            || MatchingArgumentCount == CallbackSignature::ArgumentCount,
           "Functor arguments are not compatible (must be QAudioFormat, QByteArray)");

        QtPrivate::QSlotObjectBase *slotObj = nullptr;
        if constexpr (MatchingArgumentCount == CallbackSignature::ArgumentCount) {
            slotObj = new QtPrivate::QFunctorSlotObject<Func, 2,
                typename CallbackSignature::Arguments, void>(std::move(func));
        } else if constexpr (MatchingArgumentCount == 1) {
            slotObj = new QtPrivate::QFunctorSlotObject<Func, 1,
                typename CallbackSignature::Arguments, void>(std::move(func));
        } else {
            slotObj = new QtPrivate::QFunctorSlotObject<Func, 0,
                typename QtPrivate::List_Left<void, 0>::Value, void>(std::move(func));
        }

        synthesizeImpl(text, slotObj, context);
    }

    // synthesize to a functor or function pointer (without context)
    template <typename Func, std::enable_if_t<
        !QtPrivate::FunctionPointer<Func>::IsPointerToMemberFunction
        && !std::is_same<const char *, Func>::value, bool> = true>
    void synthesize(const QString &text, Func func)
    {
        synthesize(text, nullptr, std::move(func));
    }
# endif // Q_QDOC

public Q_SLOTS:
    void say(const QString &text);
    void synthesize(const QString &text);
    void stop(QTextToSpeech::BoundaryHint boundaryHint = QTextToSpeech::BoundaryHint::Default);
    void pause(QTextToSpeech::BoundaryHint boundaryHint = QTextToSpeech::BoundaryHint::Default);
    void resume();

    void setLocale(const QLocale &locale);

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(double volume);
    void setVoice(const QVoice &voice);

Q_SIGNALS:
    void engineChanged(const QString &engine);
    void stateChanged(QTextToSpeech::State state);
    void errorOccurred(QTextToSpeech::ErrorReason error, const QString &errorString);
    void localeChanged(const QLocale &locale);
    void rateChanged(double rate);
    void pitchChanged(double pitch);
    void volumeChanged(double volume);
    void voiceChanged(const QVoice &voice);

    void sayingWord(qsizetype start, qsizetype length);
    void synthesized(const QAudioFormat &format, const QByteArray &data);

private:
    void synthesizeImpl(const QString &text,
        QtPrivate::QSlotObjectBase *slotObj, const QObject *context);

    Q_DISABLE_COPY(QTextToSpeech)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QTextToSpeech::Capabilities)

QT_END_NAMESPACE

#endif
