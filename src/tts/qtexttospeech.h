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
        Sentence,
        Utterance
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
        using CallbackSignature2 = QtPrivate::FunctionPointer<void (*)(QAudioFormat, QByteArray)>;
        using CallbackSignature1 = QtPrivate::FunctionPointer<void (*)(QAudioFormat)>;
        constexpr int MatchingArgumentCount2 = QtPrivate::ComputeFunctorArgumentCount<
            Func, CallbackSignature2::Arguments>::Value;
        constexpr int MatchingArgumentCount1 = QtPrivate::ComputeFunctorArgumentCount<
            Func, CallbackSignature1::Arguments>::Value;

        static_assert(MatchingArgumentCount2 == 0
            || MatchingArgumentCount1 == CallbackSignature1::ArgumentCount
            || MatchingArgumentCount2 == CallbackSignature2::ArgumentCount,
           "Functor arguments are not compatible (must be QAudioFormat, QByteArray)");

        QtPrivate::QSlotObjectBase *slotObj = nullptr;
        if constexpr (MatchingArgumentCount2 == CallbackSignature2::ArgumentCount) {
            slotObj = new QtPrivate::QFunctorSlotObject<Func, 2,
                typename CallbackSignature2::Arguments, void>(std::move(func));
        } else if constexpr (MatchingArgumentCount1 == CallbackSignature1::ArgumentCount) {
            slotObj = new QtPrivate::QFunctorSlotObject<Func, 1,
                typename CallbackSignature1::Arguments, void>(std::move(func));
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
    template<typename ...Args
#ifndef Q_QDOC // we need to avoid conflicts with the invokable overload
    , typename Enable = std::enable_if_t<(... && !std::is_same_v<std::decay_t<Args>, QVariantMap>)>
#endif
    >
    inline QList<QVoice> findVoices(Args &&...args) const
    {
        // if any of the arguments is a locale, then we can avoid iterating through all
        // and only have to search through the voices for that locale.
        QLocale locale;
        QLocale *plocale = nullptr;
        if constexpr (std::disjunction_v<std::is_same<std::decay_t<Args>, QLocale>...>) {
            locale = std::get<QLocale>(std::make_tuple(args...));
            plocale = &locale;
        }

        auto voices(allVoices(plocale));
        if constexpr (sizeof...(args) > 0)
            findVoicesImpl(voices, std::forward<Args>(args)...);
        return voices;
    }

    Q_INVOKABLE QList<QVoice> findVoices(const QVariantMap &criteria) const;

public Q_SLOTS:
    void say(const QString &text);
    void sayNext(const QString &text);
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
    void aboutToSynthesize(const QString &text);

private:
    void synthesizeImpl(const QString &text,
        QtPrivate::QSlotObjectBase *slotObj, const QObject *context);
    QList<QVoice> allVoices(const QLocale *locale) const;

    // Helper type to find the index of a type in a tuple, which allows
    // us to generate a compile-time error if there are multiple criteria
    // of the same type.
    template <typename T, typename Tuple> struct LastIndexOf;
    template <typename T, typename ...Ts>
    struct LastIndexOf<T, std::tuple<Ts...>> {
        template <qsizetype... Is>
        static constexpr qsizetype lastIndexOf(std::integer_sequence<qsizetype, Is...>) {
            return std::max({(std::is_same_v<T, Ts> ? Is : -1)...});
        }
        static constexpr qsizetype value =
            lastIndexOf(std::make_integer_sequence<qsizetype, sizeof...(Ts)>{});
    };
    template<typename Arg0, typename ...Args>
    inline void findVoicesImpl(QList<QVoice> &voices, Arg0 &&arg0, Args &&...args) const
    {
        using ArgType = std::decay_t<Arg0>;
        voices.removeIf([=](const auto &voice){
            if constexpr (std::is_same_v<ArgType, QLocale>) {
                return (voice.locale() != arg0);
            } else if constexpr (std::is_same_v<ArgType, QLocale::Language>) {
                return (voice.locale().language() != arg0);
            } else if constexpr (std::is_same_v<ArgType, QLocale::Territory>) {
                return (voice.locale().territory() != arg0);
            } else if constexpr (std::is_same_v<ArgType, QVoice::Gender>) {
                return (voice.gender() != arg0);
            } else if constexpr (std::is_same_v<ArgType, QVoice::Age>) {
                return (voice.age() != arg0);
            } else if constexpr (std::disjunction_v<std::is_convertible<ArgType, QString>,
                                                    std::is_convertible<ArgType, QStringView>>) {
                return (voice.name() != arg0);
            } else if constexpr (std::is_same_v<ArgType, QRegularExpression>) {
                return !arg0.match(voice.name()).hasMatch();
            } else {
                static_assert(QtPrivate::type_dependent_false<Arg0>(),
                              "Type cannot be matched to a QVoice property!");
                return true;
            }
        });
        if constexpr (sizeof...(args) > 0) {
            static_assert(LastIndexOf<ArgType, std::tuple<std::decay_t<Args>...>>::value == -1,
                          "Using multiple criteria of the same type is not supported");
            findVoicesImpl(voices, std::forward<Args>(args)...);
        }
    }

    Q_DISABLE_COPY(QTextToSpeech)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QTextToSpeech::Capabilities)

QT_END_NAMESPACE

#endif
